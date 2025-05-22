#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "test.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Qt Serial Debugger");

    // 查找当前目录下的txt文件并导入表格
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(1); // 修改列数为1
    model->setHeaderData(0, Qt::Horizontal, "名称");

    QDir directory(QCoreApplication::applicationDirPath());
    QStringList txtFiles = directory.entryList(QStringList() << "*.txt", QDir::Files);
    for (int i = 0; i < txtFiles.size(); ++i)
    {
        QList<QStandardItem *> row;
        row.append(new QStandardItem(txtFiles.at(i))); // 仅添加名称列
        model->appendRow(row);
    }

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    qDebug() << "当前目录下的txt文件导入表格完成";
    // 连接表格点击信号
    connect(ui->tableView, &QTableView::clicked, this, &MainWindow::on_tableView_clicked);

    //  状态栏
    QStatusBar *sBar = statusBar();
    // 状态栏的收、发计数标签
    lblSendNum = new QLabel(this);
    lblRecvNum = new QLabel(this);
    lblRecvRate = new QLabel(this);
    PortSelected = new QLabel(this);
    // 设置标签最小大小
    lblSendNum->setMinimumSize(100, 20);
    lblRecvNum->setMinimumSize(100, 20);
    lblRecvRate->setMinimumSize(100, 20);
    PortSelected->setMinimumSize(100, 20);

    setNumOnLabel(lblSendNum, "S: ", sendNum);
    setNumOnLabel(lblRecvNum, "R: ", recvNum);
    setNumOnLabel(lblRecvRate, "Byte/s: ", 0);
    setStrOnLabel(PortSelected, "Port: ", strPortSelected);
    // 从右往左依次添加
    sBar->addPermanentWidget(lblSendNum);
    sBar->addPermanentWidget(lblRecvNum);
    sBar->addPermanentWidget(lblRecvRate);
    sBar->addPermanentWidget(PortSelected);

    // 发送速率、接收速率统计-定时器
    timRate = new QTimer;
    timRate->start(1000);
    connect(timRate, &QTimer::timeout, this, [=]()
            { dataRateCalculate(); });

    // 新建一串口对象
    mySerialPort = new QSerialPort(this);

    // 串口接收，信号槽关联
    connect(mySerialPort, SIGNAL(readyRead()), this, SLOT(serialPortRead_Slot()));

    // 新建波形显示界面
    plot = new Plot;

    // 生成对象
    dataParser = new DataParser();

    // 添加事件过滤器以捕获键盘事件
    qApp->installEventFilter(this);

    QStringList serialPortName;

    // 自动扫描当前可用串口，返回值追加到字符数组中
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {

        // serialPortName << info.portName();// 不携带有串口设备信息的文本

        // 携带有串口设备信息的文本
        QString serialPortInfo = info.portName() + ": " + info.description(); // 串口设备信息，芯片/驱动名称
        // QString serialPortInfo = info.portName() + ": " + info.manufacturer();// 串口设备制造商
        // QString serialPortInfo = info.portName() + ": " + info.serialNumber();// 串口设备的序列号，没什么用
        // QString serialPortInfo = info.portName() + ": " + info.systemLocation();// 串口设备的系统位置，没什么用
        serialPortName << serialPortInfo;
    }
    // qDebug输出它们
    // 输出带有"Serial"字样的串口信息
    for (const QString &portInfo : serialPortName)
    {
        if (portInfo.contains("Serial", Qt::CaseInsensitive))
        {
            qDebug() << "serialPortName:" << portInfo;
            Open_Serial(portInfo);
        }
    }
}

MainWindow::~MainWindow()

{
    if (plot)
    {
        delete plot;
        plot = nullptr;
    }
    delete ui;
}
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // 获取鼠标点击位置
    QPoint clickPos = event->pos();
    // 判断点击位置是否在 QTableView 内
    if (!ui->tableView->geometry().contains(clickPos))
    {
        file_selected = NO_FILE_SELECTED;
        ui->tableView->clearSelection();
    }
    // 调用父类的鼠标按下事件处理函数
    QMainWindow::mousePressEvent(event);
}
// 绘图事件
void MainWindow::paintEvent(QPaintEvent *)
{
    // 绘图
    // 实例化画家对象，this指定绘图设备
    QPainter painter(this);

    // 设置画笔颜色
    QPen pen(QColor(0, 0, 0));
    // 设置画笔线宽（只对点线圆起作用，对文字不起作用）
    pen.setWidth(1);
    // 设置画笔线条风格，默认是SolidLine实线
    // DashLine虚线，DotLine点线，DashDotLine、DashDotDotLine点划线
    pen.setStyle(Qt::DashDotDotLine);
    // 让画家使用这个画笔
    painter.setPen(pen);

    // painter.drawLine(QPoint(ui->txtRec->x() + ui->txtRec->width(), ui->txtRec->y()), QPoint(this->width(), ui->txtRec->y()));
    // painter.drawLine(QPoint(ui->txtSend->x() + ui->txtSend->width(), ui->txtSend->y()), QPoint(this->width(), ui->txtSend->y()));
    painter.drawLine(QPoint(ui->statusbar->x(), ui->statusbar->y() - 2), QPoint(this->width(), ui->statusbar->y() - 2));
}

// 串口接收显示，槽函数
void MainWindow::serialPortRead_SlotForPlot(QByteArray recBuf)
{
    /*QString recBuf;
    recBuf = QString(mySerialPort->readAll());*/

    // QByteArray recBuf;
    //  recBuf = mySerialPort->readAll();
    //  //test
    //  recBuf = QByteArray::fromHex(ui->txtRec->toPlainText().remove(" ").toUtf8());
    //  qDebug() << "Received Data:" << recBuf;
    // test
    /* 帧过滤部分代码 */
    short wmValue[20] = {0};
    xFrameDataFilter(&recBuf, wmValue);

    // 调试信息输出，显示缓冲区内容（16进制显示）及接收标志位
    // if(!ui->widget_5->isHidden()){
    //     QByteArray str1;
    //     //for(int i=0; i<(tnum + 1); i++)
    //     for(int i=0; i<BufferSize; i++)
    //     {
    //         str1.append(chrtmp[i]);
    //     }
    //     //ui->txtFrameTemp->setPlainText(str1.toHex().toUpper());
    //     str1 = str1.toHex().toUpper();
    //     QString str2;
    //     for(int i = 0; i<str1.length (); i+=2)
    //     {
    //         str2 += str1.mid (i,2);
    //         str2 += " ";
    //     }
    //     ui->txtFrameBuffer->setPlainText(str2);
    //     // 显示标志位
    //     ui->txtFrameTnum->setText(QString::number(tnum));
    //     ui->txtFrameH1->setText(QString::number(f_h1_flag));
    //     ui->txtFrameH->setText(QString::number(f_h_flag));
    //     ui->txtFrameFun->setText(QString::number(f_fun_word));
    //     ui->txtFrameLen->setText(QString::number(f_length));
    //     ui->txtFrameErrorNum->setText(QString::number(recvErrorNum));
    //}

    // 接收字节计数
    recvNum += recBuf.size();
    // 状态栏显示计数值
    setNumOnLabel(lblRecvNum, "R: ", recvNum);

    // 判断是否为16进制接收，将以后接收的数据全部转换为16进制显示（先前接收的部分在多选框槽函数中进行转换。最好多选框和接收区组成一个自定义控件，方便以后调用）
    if (ui->chkRec->checkState() == false)
    {
        // GB2312编码输入
        QString strb = QString::fromLocal8Bit(recBuf); // QString::fromUtf8(recBuf);//QString::fromLatin1(recBuf);
        // 在当前位置插入文本，不会发生换行。如果没有移动光标到文件结尾，会导致文件超出当前界面显示范围，界面也不会向下滚动。
        ui->txtRec->insertPlainText(strb);
    }
    else
    {
        // 16进制显示，并转换为大写
        QString str1 = recBuf.toHex().toUpper(); //.data();
        // 添加空格
        QString str2;
        for (int i = 0; i < str1.length(); i += 2)
        {
            str2 += str1.mid(i, 2);
            str2 += " ";
        }
        ui->txtRec->insertPlainText(str2);
        // ui->txtRec->insertPlainText(recBuf.toHex());
    }

    // 移动光标到文本结尾
    ui->txtRec->moveCursor(QTextCursor::End);

    // 将文本追加到末尾显示，会导致插入的文本换行
    /*ui->txtRec->appendPlainText(recBuf);*/

    /*// 在当前位置插入文本，不会发生换行。如果没有移动光标到文件结尾，会导致文件超出当前界面显示范围，界面也不会向下滚动。
    ui->txtRec->insertPlainText(recBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/

    // 利用一个QString去获取消息框文本，再将新接收到的消息添加到QString尾部，但感觉效率会比当前位置插入低。也不会发生换行
    /*QString txtBuf;
    txtBuf = ui->txtRec->toPlainText();
    txtBuf += recBuf;
    ui->txtRec->setPlainText(txtBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/

    // 利用一个QString去缓存接收到的所有消息，效率会比上面高一点。但清空接收的时候，要将QString一并清空。
    /*static QString txtBuf;
    txtBuf += recBuf;
    ui->txtRec->setPlainText(txtBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/
}

void MainWindow::serialPortRead_Slot()
{
    /*QString recBuf;
    recBuf = QString(mySerialPort->readAll());*/

    QByteArray recBuf;
    recBuf = mySerialPort->readAll();

    /* 帧过滤部分代码 */
    // short wmValue[20] = {0};
    //  xFrameDataFilter(&recBuf, wmValue);

    // 调试信息输出，显示缓冲区内容（16进制显示）及接收标志位
    //    if (!ui->widget_5->isHidden())
    //    {
    //        QByteArray str1;
    //        // for(int i=0; i<(tnum + 1); i++)
    //        for (int i = 0; i < BufferSize; i++)
    //        {
    //            str1.append(chrtmp[i]);
    //        }
    //        // ui->txtFrameTemp->setPlainText(str1.toHex().toUpper());
    //        str1 = str1.toHex().toUpper();
    //        QString str2;
    //        for (int i = 0; i < str1.length(); i += 2)
    //        {
    //            str2 += str1.mid(i, 2);
    //            str2 += " ";
    //        }
    //        //ui->txtFrameBuffer->setPlainText(str2);
    //        // 显示标志位
    //        ui->txtFrameTnum->setText(QString::number(tnum));
    //        ui->txtFrameH1->setText(QString::number(f_h1_flag));
    //        ui->txtFrameH->setText(QString::number(f_h_flag));
    //        ui->txtFrameFun->setText(QString::number(f_fun_word));
    //        ui->txtFrameLen->setText(QString::number(f_length));
    //        ui->txtFrameErrorNum->setText(QString::number(recvErrorNum));
    //    }

    // 接收字节计数
    recvNum += recBuf.size();
    // 状态栏显示计数值
    setNumOnLabel(lblRecvNum, "R: ", recvNum);

    // 判断是否为16进制接收，将以后接收的数据全部转换为16进制显示（先前接收的部分在多选框槽函数中进行转换。最好多选框和接收区组成一个自定义控件，方便以后调用）
    if (ui->chkRec->checkState() == false)
    {
        // GB2312编码输入
        QString strb = QString::fromLocal8Bit(recBuf); // QString::fromUtf8(recBuf);//QString::fromLatin1(recBuf);
        // 在当前位置插入文本，不会发生换行。如果没有移动光标到文件结尾，会导致文件超出当前界面显示范围，界面也不会向下滚动。
        ui->txtRec->insertPlainText(strb);
    }
    else
    {
        // 16进制显示，并转换为大写
        QString str1 = recBuf.toHex().toUpper(); //.data();
        // 添加空格
        QString str2;
        for (int i = 0; i < str1.length(); i += 2)
        {
            str2 += str1.mid(i, 2);
            str2 += " ";
        }
        ui->txtRec->insertPlainText(str2);
        // ui->txtRec->insertPlainText(recBuf.toHex());
    }

    // 移动光标到文本结尾
    ui->txtRec->moveCursor(QTextCursor::End);

    // 将文本追加到末尾显示，会导致插入的文本换行
    /*ui->txtRec->appendPlainText(recBuf);*/

    /*// 在当前位置插入文本，不会发生换行。如果没有移动光标到文件结尾，会导致文件超出当前界面显示范围，界面也不会向下滚动。
    ui->txtRec->insertPlainText(recBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/

    // 利用一个QString去获取消息框文本，再将新接收到的消息添加到QString尾部，但感觉效率会比当前位置插入低。也不会发生换行
    /*QString txtBuf;
    txtBuf = ui->txtRec->toPlainText();
    txtBuf += recBuf;
    ui->txtRec->setPlainText(txtBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/

    // 利用一个QString去缓存接收到的所有消息，效率会比上面高一点。但清空接收的时候，要将QString一并清空。
    /*static QString txtBuf;
    txtBuf += recBuf;
    ui->txtRec->setPlainText(txtBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/
}

// 打开/关闭串口 槽函数
void MainWindow::on_btnSwitch_clicked()
{
    QString spTxt = ui->cmbSerialPort->currentText();

    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity checkBits;

    // 获取串口波特率
    baudRate = (QSerialPort::BaudRate)ui->cmbBaudRate->currentText().toUInt();
    // 获取串口数据位
    dataBits = (QSerialPort::DataBits)ui->cmbData->currentText().toUInt();
    // 获取串口停止位
    if (ui->cmbStop->currentText() == "1")
    {
        stopBits = QSerialPort::OneStop;
    }
    else if (ui->cmbStop->currentText() == "1.5")
    {
        stopBits = QSerialPort::OneAndHalfStop;
    }
    else if (ui->cmbStop->currentText() == "2")
    {
        stopBits = QSerialPort::TwoStop;
    }
    else
    {
        stopBits = QSerialPort::OneStop;
    }

    // 获取串口奇偶校验位
    if (ui->cmbCheck->currentText() == "无")
    {
        checkBits = QSerialPort::NoParity;
    }
    else if (ui->cmbCheck->currentText() == "奇校验")
    {
        checkBits = QSerialPort::OddParity;
    }
    else if (ui->cmbCheck->currentText() == "偶校验")
    {
        checkBits = QSerialPort::EvenParity;
    }
    else
    {
        checkBits = QSerialPort::NoParity;
    }

    // 想想用 substr strchr怎么从带有信息的字符串中提前串口号字符串
    // 初始化串口属性，设置 端口号、波特率、数据位、停止位、奇偶校验位数
    mySerialPort->setBaudRate(baudRate);
    mySerialPort->setDataBits(dataBits);
    mySerialPort->setStopBits(stopBits);
    mySerialPort->setParity(checkBits);
    // mySerialPort->setPortName(ui->cmbSerialPort->currentText());// 不匹配带有串口设备信息的文本
    //  匹配带有串口设备信息的文本

    spTxt = spTxt.section(':', 0, 0); // spTxt.mid(0, spTxt.indexOf(":"));
    // qDebug() << spTxt;
    mySerialPort->setPortName(spTxt);

    // 根据初始化好的串口属性，打开串口
    // 如果打开成功，反转打开按钮显示和功能。打开失败，无变化，并且弹出错误对话框。
    if (ui->btnSwitch->text() == "打开串口")
    {
        if (mySerialPort->open(QIODevice::ReadWrite) == true)
        {
            // QMessageBox::
            ui->btnSwitch->setText("关闭串口");
            // 让端口号下拉框不可选，避免误操作（选择功能不可用，控件背景为灰色）
            ui->cmbSerialPort->setEnabled(false);
            ui->cmbBaudRate->setEnabled(false);
            ui->cmbStop->setEnabled(false);
            ui->cmbData->setEnabled(false);
            ui->cmbCheck->setEnabled(false);
        }
        else
        {
            QMessageBox::critical(this, "错误提示", "串口打开失败！！！\r\n\r\n该串口可能被占用，请选择正确的串口\r\n或者波特率过高，超出硬件支持范围");
        }
        // 收起串口配置
        on_inandoutButton_released();
        // 设置状态栏显示串口号
        setStrOnLabel(PortSelected, "Port: ", spTxt);
    }

    else
    {
        mySerialPort->close();
        ui->btnSwitch->setText("打开串口");
        // 端口号下拉框恢复可选，避免误操作
        ui->cmbSerialPort->setEnabled(true);
        ui->cmbBaudRate->setEnabled(true);
        ui->cmbStop->setEnabled(true);
        ui->cmbData->setEnabled(true);
        ui->cmbCheck->setEnabled(true);
        // 设置状态栏显示串口号
        setStrOnLabel(PortSelected, "Port: ", "NoPort");
    }
}

void MainWindow::Open_Serial(QString spTxt)
{

    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity checkBits;

    // 获取串口波特率
    baudRate = (QSerialPort::BaudRate)ui->cmbBaudRate->currentText().toUInt();
    // 获取串口数据位
    dataBits = (QSerialPort::DataBits)ui->cmbData->currentText().toUInt();
    // 获取串口停止位
    if (ui->cmbStop->currentText() == "1")
    {
        stopBits = QSerialPort::OneStop;
    }
    else if (ui->cmbStop->currentText() == "1.5")
    {
        stopBits = QSerialPort::OneAndHalfStop;
    }
    else if (ui->cmbStop->currentText() == "2")
    {
        stopBits = QSerialPort::TwoStop;
    }
    else
    {
        stopBits = QSerialPort::OneStop;
    }

    // 获取串口奇偶校验位
    if (ui->cmbCheck->currentText() == "无")
    {
        checkBits = QSerialPort::NoParity;
    }
    else if (ui->cmbCheck->currentText() == "奇校验")
    {
        checkBits = QSerialPort::OddParity;
    }
    else if (ui->cmbCheck->currentText() == "偶校验")
    {
        checkBits = QSerialPort::EvenParity;
    }
    else
    {
        checkBits = QSerialPort::NoParity;
    }

    // 想想用 substr strchr怎么从带有信息的字符串中提前串口号字符串
    // 初始化串口属性，设置 端口号、波特率、数据位、停止位、奇偶校验位数
    mySerialPort->setBaudRate(baudRate);
    mySerialPort->setDataBits(dataBits);
    mySerialPort->setStopBits(stopBits);
    mySerialPort->setParity(checkBits);
    // mySerialPort->setPortName(ui->cmbSerialPort->currentText());// 不匹配带有串口设备信息的文本
    //  匹配带有串口设备信息的文本

    spTxt = spTxt.section(':', 0, 0); // spTxt.mid(0, spTxt.indexOf(":"));
    // qDebug() << spTxt;
    mySerialPort->setPortName(spTxt);

    // 根据初始化好的串口属性，打开串口
    // 如果打开成功，反转打开按钮显示和功能。打开失败，无变化，并且弹出错误对话框。
    if (ui->btnSwitch->text() == "打开串口")
    {
        if (mySerialPort->open(QIODevice::ReadWrite) == true)
        {
            // QMessageBox::
            ui->btnSwitch->setText("关闭串口");
            // 让端口号下拉框不可选，避免误操作（选择功能不可用，控件背景为灰色）
            ui->cmbSerialPort->setEnabled(false);
            ui->cmbBaudRate->setEnabled(false);
            ui->cmbStop->setEnabled(false);
            ui->cmbData->setEnabled(false);
            ui->cmbCheck->setEnabled(false);
        }
        else
        {
            QMessageBox::critical(this, "错误提示", "串口打开失败！！！\r\n\r\n该串口可能被占用，请选择正确的串口\r\n或者波特率过高，超出硬件支持范围");
        }
    }
    else
    {
        mySerialPort->close();
        ui->btnSwitch->setText("打开串口");
        // 端口号下拉框恢复可选，避免误操作
        ui->cmbSerialPort->setEnabled(true);
        ui->cmbBaudRate->setEnabled(true);
        ui->cmbStop->setEnabled(true);
        ui->cmbData->setEnabled(true);
        ui->cmbCheck->setEnabled(true);
    }
    // 收起串口配置
    on_inandoutButton_released();

    setStrOnLabel(PortSelected, "Port: ", spTxt);
}

// 状态栏标签显示计数值
void MainWindow::setNumOnLabel(QLabel *lbl, QString strS, long num)
{
    // 标签显示
    QString strN;
    strN.sprintf("%ld", num);
    QString str = strS + strN;
    lbl->setText(str);
}
// 状态栏标签显示计数值
void MainWindow::setStrOnLabel(QLabel *lbl, QString strS, QString strIn)
{
    // 标签显示
    QString strN;
    strN = strIn;
    QString str = strS + strN;
    lbl->setText(str);
}

void MainWindow::LabelUpdate(void)
{
}

void MainWindow::on_btnClearRec_clicked()
{
    ui->txtRec->clear();
    // 清除发送、接收字节计数
    sendNum = 0;
    recvNum = 0;
    tSend = 0;
    tRecv = 0;
    // 状态栏显示计数值
    setNumOnLabel(lblSendNum, "S: ", sendNum);
    setNumOnLabel(lblRecvNum, "R: ", recvNum);
    // 清空帧数量
    recvFrameRate = 0, recvErrorNum = 0, tFrame = 0;

    // ui->txtFrameErrorNum->setText(QString::number(recvErrorNum));
}

// 先前接收的部分在多选框状态转换槽函数中进行转换。（最好多选框和接收区组成一个自定义控件，方便以后调用）
void MainWindow::on_chkRec_stateChanged(int arg1)
{
    // 获取文本字符串
    QString txtBuf = ui->txtRec->toPlainText();

    // 获取多选框状态，未选为0，选中为2
    // 为0时，多选框未被勾选，接收区先前接收的16进制数据转换为asc2字符串格式
    if (arg1 == 0)
    {

        // QString str1 = QByteArray::fromHex(txtBuf.toUtf8());
        // QString str1 = QByteArray::fromHex(txtBuf.toLocal8Bit());
        // 把gb2312编码转换成unicode
        QString str1 = QTextCodec::codecForName("GB2312")->toUnicode(QByteArray::fromHex(txtBuf.toLocal8Bit()));
        // 文本控件清屏，显示新文本
        ui->txtRec->clear();
        ui->txtRec->insertPlainText(str1);
        // 移动光标到文本结尾
        ui->txtRec->moveCursor(QTextCursor::End);
    }
    else
    { // 不为0时，多选框被勾选，接收区先前接收asc2字符串转换为16进制显示

        // QString str1 = txtBuf.toUtf8().toHex().toUpper();// Unicode编码输出
        QString str1 = txtBuf.toLocal8Bit().toHex().toUpper(); // GB2312编码输出
        // 添加空格
        QByteArray str2;
        for (int i = 0; i < str1.length(); i += 2)
        {
            str2 += str1.mid(i, 2);
            str2 += " ";
        }
        // 文本控件清屏，显示新文本
        ui->txtRec->clear();
        ui->txtRec->insertPlainText(str2);
        // 移动光标到文本结尾
        ui->txtRec->moveCursor(QTextCursor::End);
    }
}

// 发送速率、接收速率统计-定时器
void MainWindow::dataRateCalculate(void)
{
    recvRate = recvNum - tRecv; // * ui->cmbData->currentText().toUInt();

    setNumOnLabel(lblRecvRate, "Byte/s: ", recvRate);
    tSend = sendNum;
    tRecv = recvNum;
}

void MainWindow::dataShow(void)
{
    qDebug() << "dataShow";
}
// "显示波形界面" 按键槽函数
// 记得把plot在析构中释放掉，不然很容易下次运行崩溃
void MainWindow::on_pushButton_clicked()
{
    // 从串口接收数据
    receivedData = QByteArray::fromHex(ui->txtRec->toPlainText().toUtf8());
    // 删除现有的 plot 对象
    if (plot)
    {
        delete plot;
        plot = nullptr;
    }

    // 创建一个新的 Plot 对象
    plot = new Plot;

    // 显示新的波形绘图窗口
    plot->show();
    // 修改plot的标题

    int rowIndex = file_selected;
    QString secondColumnData = ui->tableView->model()->index(rowIndex, 0).data().toString();
    qDebug() << "Second column content:" << secondColumnData;
    // QString plotTitle = "波形显示";
    plot->setWindowTitle(secondColumnData);
    for (int i = 0; i < receivedData.size(); i += 9)
    {
        sendDataFrame = receivedData.mid(i, 9);
        // qDebug() << "sendDataFrame:" << sendDataFrame.toHex().toUpper();
        //  int bytesWritten = mySerialPort->write(chunk); // 发送数据
        //  if (bytesWritten > 0) {
        //      sendNum += bytesWritten; // 更新发送字节计数
        //      setNumOnLabel(lblSendNum, "S: ", sendNum); // 更新状态栏显示
        //  }
        //        if (1)
        //        {
        //            for (int i = 0; i < 2; i++)
        //            {
        //                value[i] = ((short)chrtmp[i * 2 + 4] << 8) | chrtmp[i * 2 + 4 + 1];
        //            }
        //        }

        //        // 显示波形（在这里显示可以处理多帧粘包，避免多帧粘包只显示一帧的情况）
        //        // 将解析出的short数组，传入波形图，进行绘图
        //        if (!plot->isHidden())
        //        {
        //            plot->ShowPlot_WaveForm(plot->pPlot1, value);
        //        }
        serialPortRead_SlotForPlot(sendDataFrame);
    }
}

/*
void MainWindow::xFrameDataFilter(QByteArray *str)
{
    int num = str->size();
    if(num)
    {

    }
}*/

// 帧过滤
// 适用于有帧头、功能字、有效字段长度、校验位的接收，无帧尾
void MainWindow::xFrameDataFilter(QByteArray *str, short value[])
{
    int num = str->size();
    qDebug() << "The size of the QByteArray is:" << num;
    if (num)
    {
        for (int i = 0; i < num; i++)
        {
            chrtmp[tnum] = str->at(i); // 从接收缓存区读取一个字节
            if (f_h_flag == 1)         // 有帧头。判断功能字、有效字段长度，接收消息
            {
                if (f_fun_word) // 有帧头，有功能字
                {
                    if (f_length) // 有帧头，有功能字，有有效字节长度
                    {
                        if ((tnum - 4) < f_length) // 有帧头，有功能字，未超出有效字节长度+校验位，接收数据
                        {
                            tnum++;
                        }
                        else // 有帧头，有功能字，超出有效字节长度。判断校验位
                        {
                            // 累加和校验计算
                            unsigned char crc = 0;
                            for (i = 0; i < tnum; i++)
                            {
                                crc += chrtmp[i];
                            }

                            // 校验对比
                            if (crc == chrtmp[tnum]) // 校验通过，将缓冲区的数据打包发送
                            {

                                // 调试信息输出，显示有效帧的内容（16进制显示）
                                //                                if (!ui->widget_5->isHidden())
                                //                                {
                                //                                    QByteArray str1;
                                //                                    for (int i = 0; i < (tnum + 1); i++)
                                //                                    {
                                //                                        str1.append(chrtmp[i]);
                                //                                    }
                                //                                    // ui->txtFrameEffective->appendPlainText(str1.toHex().toUpper());
                                //                                    str1 = str1.toHex().toUpper();
                                //                                    QString str2;
                                //                                    for (int i = 0; i < str1.length(); i += 2)
                                //                                    {
                                //                                        str2 += str1.mid(i, 2);
                                //                                        str2 += " ";
                                //                                    }
                                //                                    ui->txtFrameEffective->appendPlainText(str2);
                                //                                }

                                // 根据功能字进行功能解析，自动根据帧长度解析为对应的short值。
                                if (f_fun_word == FunWord_WF)
                                {
                                    for (int i = 0; i < (f_length / 2); i++)
                                    {
                                        value[i] = ((short)chrtmp[i * 2 + 4] << 8) | chrtmp[i * 2 + 4 + 1];
                                    }
                                }

                                // 显示波形（在这里显示可以处理多帧粘包，避免多帧粘包只显示一帧的情况）
                                // 将解析出的short数组，传入波形图，进行绘图
                                if (!plot->isHidden())
                                {
                                    plot->ShowPlot_WaveForm(plot->pPlot1, value);
                                }
                                // qDebug() << "test4, i:" << i;
                            }
                            else
                            {
                                ++recvErrorNum; // 误码帧数量计数
                            }

                            // 清0重新接收
                            tnum = 0;
                            // 清空标志位
                            f_h1_flag = 0;
                            f_h_flag = 0;
                            f_fun_word = 0;
                            f_length = 0;
                        }
                        // 把上面下面的判断标志位 == 1去掉

                    } // 有帧头，有功能字，判断是否是有效字节长度
                    else
                    {
                        if (chrtmp[tnum] <= ValidByteLength)
                        {
                            f_length = chrtmp[tnum]; // 记录当前帧的有效字节长度
                            tnum++;
                        }
                        else
                        {
                            // 清0重新接收
                            tnum = 0;
                            // 清空标志位
                            f_h1_flag = 0;
                            f_h_flag = 0;
                            f_fun_word = 0;
                        }
                    }
                }
                else // 有帧头，无功能字，判断是否为有效功能字
                {
                    if ((chrtmp[tnum] == FunWord_WF) || chrtmp[tnum] == FunWord_SM)
                    {
                        f_fun_word = chrtmp[tnum]; // 记录功能字
                        tnum++;
                    }
                    else
                    {
                        // 清0重新接收
                        tnum = 0;
                        // 清空标志位
                        f_h1_flag = 0;
                        f_h_flag = 0;
                    }
                }
            }
            else // 没有接收到帧头
            {
                if (f_h1_flag == 1) // 没有帧头，有帧头1。下一步判断是否为第2个字节
                {
                    if (chrtmp[tnum] == Frame_Header2) // 如果为帧头的第2个字节，接收到帧头标志位标志位置1，tnum自增
                    {
                        f_h_flag = 1;
                        tnum++;
                    }
                    else
                    {
                        // 这里再添加一个判断，出现 3A 3A 3B xx的情况，如果没有这个判断会重新计数，导致丢帧
                        if (chrtmp[tnum] == Frame_Header1)
                        {
                            f_h1_flag = 1;
                            tnum = 1;
                        }
                        else
                        {
                            // 重新计数，但如果出现 3A 3A 3B xx的情况，会导致丢帧，要加上上面的判断
                            f_h1_flag = 0;
                            tnum = 0;
                        }
                    }
                }
                else // 没有帧头，没有有帧头1。下一步判断，是否为帧头的第1个字节
                {
                    if (chrtmp[tnum] == Frame_Header1) // 如果为帧头的第1个字节，标志位置1，tnum自增
                    {
                        f_h1_flag = 1;
                        tnum++;
                    }
                    else // 否则，标志位清0，tnum清0
                    {
                        tnum = 0;
                    }
                }
            }

            // 2.判断多长的数据没有换行符，如果超过2000，会人为向数据接收区添加换行，来保证CPU占用率不会过高，不会导致卡顿
            //   但由于是先插入换行，后插入接收到的数据，所以每一箩数据并不是2000
            static int xx = 0;
            if (chrtmp[tnum] != 0x0A)
            {
                ++xx;
                if (xx > 2000)
                {
                    ui->txtRec->appendPlainText("");
                    ui->txtRec->appendPlainText("");
                    xx = 0;
                }
            }
            else
            {
                xx = 0;
            }

            // 3.超过数据帧最大长度的不要，最多同时显示20条曲线。
            //   大于MaxFrameLength个字节的帧不接收
            if (tnum > (MaxFrameLength - 1))
            {
                tnum = 0;
                f_h1_flag = 0;
                f_h_flag = 0;
                f_t1_flag = 0;
                // f_fun_word = 0;
                // f_length = 0;
                continue;
            }
        }
        qDebug() << "testtest";
    }
}

/*
// 适用于有帧头帧尾、无功能字和有效字段长度的接收
void MainWindow::xFrameDataFilter(QByteArray *str)
{
    int num = str->size();
    if(num)
    {
        for(int i=0; i<num; i++)
        {
            chrtmp[tnum] = str->at(i);  		// 从接收缓存区读取一个字节
            if (f_h_flag == 1)  // 有帧头，判断帧尾，接收消息
            {
                if (f_t1_flag == 1) //有帧头，有帧尾1
                {
                    if (chrtmp[tnum] == Frame_Tail2)
                    {
                        tnum ++;

                        // 接收到一帧有效帧
                        // 用户处理代码 //
                        // 根据接收到的数量，合成帧字段
                        // 调试信息输出，显示有效帧的内容（16进制显示）
                        if(!ui->widget_5->isHidden()){
                            QByteArray str1;
                            for(int i=0; i<tnum; i++)
                            {
                                str1.append(chrtmp[i]);
                            }
                            //ui->txtFrameEffective->appendPlainText(str1.toHex().toUpper());
                            str1 = str1.toHex().toUpper();
                            QString str2;
                            for(int i = 0; i<str1.length (); i+=2)
                            {
                                str2 += str1.mid (i,2);
                                str2 += " ";
                            }
                            ui->txtFrameEffective->appendPlainText(str2);
                        }

                        //  处理完用户代码，重新接收计数 //
                        tnum = 0;
                        // 清空标志位，之前一直忘了
                        f_h1_flag = 0;
                        f_h_flag = 0;
                        f_t1_flag = 0;

                        // 将接收到符合帧定义的帧，原路发送回去 //
                        //return str1;
                        //ui->lineEdit->setText(str1.toHex().toUpper());

                    }
                    else
                    {
                        f_t1_flag = 0;
                        tnum ++;
                    }
                }
                else						// 有帧头，无帧尾1
                {
                    if (chrtmp[tnum] == Frame_Tail1)
                    {
                        f_t1_flag = 1;
                        tnum ++;
                    }
                    else					// 接收消息包中间内容
                    {
                        tnum ++;
                    }
                }
            }
            else						// 没有接收到帧头
            {
                if (f_h1_flag == 1)			        //没有帧头，有帧头1。下一步判断是否为第2个字节
                {
                    if (chrtmp[tnum] == Frame_Header2)          // 如果为帧头的第2个字节，接收到帧头标志位标志位置1，tnum自增
                    {
                        f_h_flag = 1;
                        tnum ++;
                    }
                    else
                    {
                        // 这里再添加一个判断，出现 3A 3A 3B xx的情况，如果没有这个判断会重新计数，导致丢帧
                        if(chrtmp[tnum] == Frame_Header1){
                            f_h1_flag = 1;
                            tnum = 1;
                        }else{
                            // 重新计数，但如果出现 3A 3A 3B xx的情况，会导致丢帧，要加上上面的判断
                            f_h1_flag = 0;
                            tnum = 0;
                        }
                    }
                }
                else						//没有帧头，没有有帧头1。下一步判断，是否为帧头的第1个字节
                {
                    if (chrtmp[tnum] == Frame_Header1)  // 如果为帧头的第1个字节，标志位置1，tnum自增
                    {
                        f_h1_flag = 1;
                        tnum ++;
                    }
                    else                                // 否则，标志位清0，tnum清0
                    {
                        tnum = 0;
                    }
                }
            }

            // 大于MaxFrameLength个字节的帧不接收
            if (tnum > (MaxFrameLength - 1) )
            {
                tnum = 0;
                f_h1_flag = 0;
                f_h_flag = 0;
                f_t1_flag = 0;
                continue;
            }
        }
    }
}*/

// 测试1
void MainWindow::on_pushButton_3_released()
{
    int GroupCount = 0;
    int group_count = 0;
    int idx_index = 0; // 每组的起始点
    memset(logData, 0, sizeof(logData));

    qDebug() << "查找每两个\"EVENT: 201\"之间的内容";
    QString plainText = ui->txtRec->toPlainText();
    QStringList lines = plainText.split('\n');

    QList<int> eventIndices;
    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines[i].contains("EVENT: 201", Qt::CaseInsensitive))
        {
            eventIndices << i;
        }
    }

    if (eventIndices.size() < 2)
    {
        if (eventIndices.size() == 1)
        {
            // 只有一个EVENT: 201，输出它到结尾
            QStringList betweenLines;
            for (int i = eventIndices[0] + 1; i < lines.size(); ++i)
                betweenLines << lines[i];
            qDebug() << "最后一个\"EVENT: 201\"到结尾的内容:";
            for (const QString &line : betweenLines)
                qDebug() << line;
        }
        else
        {
            qDebug() << "未找到\"EVENT: 201\"或只有一个";
        }
        return;
    }
    int idx = 0;
    // 输出每两个EVENT: 201之间的内容
    for (; idx < eventIndices.size() - 1; ++idx)
    {
        int start = eventIndices[idx];
        int end = eventIndices[idx + 1];
        if (end > start + 1)
        {
            QStringList betweenLines;
            for (int i = start + 1; i < end; ++i)
                betweenLines << lines[i];
            qDebug() << QString("第%1对\"EVENT: 201\"之间的内容:").arg(idx + 1);
            int innerGroupCount = 0;
            GroupCount = 0;
            for (const QString &line : betweenLines)
            {
                QStringList items = line.split(' ', QString::SkipEmptyParts);
                if (items.size() >= 11)
                {
                    innerGroupCount++;
                    logData[idx_index + innerGroupCount].id = items[0].toInt();
                    logData[idx_index + innerGroupCount].timestamp = items[1].toInt();
                    logData[idx_index + innerGroupCount].currentMode = items[2].toInt();
                    logData[idx_index + innerGroupCount].phaseFlag = items[4].toInt();
                    logData[idx_index + innerGroupCount].goDestSpeed = items[5].toInt();
                    logData[idx_index + innerGroupCount].firstPhaseCount = items[6].toInt();
                    logData[idx_index + innerGroupCount].originBearing = items[7].toFloat();
                    logData[idx_index + innerGroupCount].currentBearing = items[8].toFloat();
                    logData[idx_index + innerGroupCount].currentYaw = items[9].toFloat();
                    logData[idx_index + innerGroupCount].currentDistance = items[10].toFloat();
                    logData[idx_index + innerGroupCount].lineSeparation = items[11].toFloat();
                    logData[idx_index + innerGroupCount].bearingError = items[12].toFloat();
                    logData[idx_index + innerGroupCount].yawCurrentBearing = items[13].toFloat();
                    logData[idx_index + innerGroupCount].rudderAngle = items[14].toFloat();
                    logData[idx_index + innerGroupCount].motorSpeedLeft = items[15].toInt();
                    logData[idx_index + innerGroupCount].motorSpeedRight = items[16].toInt();
                    qDebug() << "currentDistance:" << logData[idx_index + innerGroupCount].currentDistance << "lineSeparation:" << logData[idx_index + innerGroupCount].lineSeparation << "rudderAngle:" << logData[idx_index + innerGroupCount].rudderAngle << "motorSpeedLeft:" << logData[idx_index + innerGroupCount].motorSpeedLeft << "motorSpeedRight:" << logData[idx_index + innerGroupCount].motorSpeedRight;
                    GroupCount++;
                }
            }
            group_index[group_count].first = idx_index;
            group_index[group_count].second = innerGroupCount;
            group_count++;
            idx_index += (innerGroupCount + 10);
            qDebug() << "innerGroupCount:" << innerGroupCount;
            qDebug() << QString("第%1对\"EVENT: 201\"之间的内容的数量:").arg(GroupCount);
        }
    }
    // 如果EVENT: 201数量为奇数，输出最后一个到结尾
    if (eventIndices.size() % 2 == 1)
    {
        int last = eventIndices.last();
        if (last + 1 < lines.size())
        {
            QStringList betweenLines;
            for (int i = last + 1; i < lines.size(); ++i)
                betweenLines << lines[i];
            qDebug() << QString("第%1对\"EVENT: 201\"之间的内容:").arg(idx + 1);
            int innerGroupCount = 0;

            for (const QString &line : betweenLines)
            {
                QStringList items = line.split(' ', QString::SkipEmptyParts);
                if (items.size() >= 11)
                {
                    innerGroupCount++;
                    logData[idx_index + innerGroupCount].id = items[0].toInt();
                    logData[idx_index + innerGroupCount].timestamp = items[1].toInt();
                    logData[idx_index + innerGroupCount].currentMode = items[2].toInt();
                    logData[idx_index + innerGroupCount].phaseFlag = items[4].toInt();
                    logData[idx_index + innerGroupCount].goDestSpeed = items[5].toInt();
                    logData[idx_index + innerGroupCount].firstPhaseCount = items[6].toInt();
                    logData[idx_index + innerGroupCount].originBearing = items[7].toFloat();
                    logData[idx_index + innerGroupCount].currentBearing = items[8].toFloat();
                    logData[idx_index + innerGroupCount].currentYaw = items[9].toFloat();
                    logData[idx_index + innerGroupCount].currentDistance = items[10].toFloat();
                    logData[idx_index + innerGroupCount].lineSeparation = items[11].toFloat();
                    logData[idx_index + innerGroupCount].bearingError = items[12].toFloat();
                    logData[idx_index + innerGroupCount].yawCurrentBearing = items[13].toFloat();
                    logData[idx_index + innerGroupCount].rudderAngle = items[14].toFloat();
                    logData[idx_index + innerGroupCount].motorSpeedLeft = items[15].toInt();
                    logData[idx_index + innerGroupCount].motorSpeedRight = items[16].toInt();
                    qDebug() << "currentDistance:" << logData[idx_index + innerGroupCount].currentDistance << "lineSeparation:" << logData[idx_index + innerGroupCount].lineSeparation << "rudderAngle:" << logData[idx_index + innerGroupCount].rudderAngle << "motorSpeedLeft:" << logData[idx_index + innerGroupCount].motorSpeedLeft << "motorSpeedRight:" << logData[idx_index + innerGroupCount].motorSpeedRight;
                    GroupCount++;
                }
            }
            group_index[group_count].first = idx_index;
            group_index[group_count].second = innerGroupCount;
            qDebug() << QString("最后一组\"EVENT: 201\"到结尾的内容的数量: %1").arg(GroupCount);
        }
    }

    for (int i = 0; i <= group_count; i++)
    {
        qDebug() << "group_index[" << i << "]:" << group_index[i].first << "," << group_index[i].second;
    }
    QStringList groupOptions;
    for (int i = 0; i < group_count; ++i)
    {
        groupOptions << QString("第%1组").arg(i + 1);
    }
    bool ok = false;
    QString selectedGroup = QInputDialog::getItem(this, "选择分组", "请选择一个分组：", groupOptions, 0, false, &ok);
    if (ok && !selectedGroup.isEmpty())
    {
        int selectedIndex = groupOptions.indexOf(selectedGroup);
        qDebug() << "用户选择了分组:" << selectedGroup << "，索引:" << selectedIndex;
        // 这里可以根据 selectedIndex 进行后续处理
        if (plot)
        {
            delete plot;
            plot = nullptr;
        }

        // 创建一个新的 Plot 对象
        plot = new Plot;

        // 显示新的波形绘图窗口
        plot->showMaximized();
        // 修改plot的标题

        int rowIndex = file_selected;
        QString secondColumnData = ui->tableView->model()->index(rowIndex, 0).data().toString();
        qDebug() << "Second column content:" << secondColumnData;
        // QString plotTitle = "波形显示";
        plot->setWindowTitle(secondColumnData);
        float value[20] = {0};
        for (int j = 1; j <= group_index[selectedIndex].second; j++)
        {
            value[0] = logData[group_index[selectedIndex].first + j].currentDistance;
            value[1] = logData[group_index[selectedIndex].first + j].lineSeparation;
            value[2] = logData[group_index[selectedIndex].first + j].rudderAngle;
            value[3] = logData[group_index[selectedIndex].first + j].motorSpeedLeft;
            value[4] = logData[group_index[selectedIndex].first + j].motorSpeedRight;
            
            // 输出一下
            qDebug() << "currentDistance:" << value[0] << "lineSeparation:" << value[1] << "rudderAngle:" << value[2] << "motorSpeedLeft:" << value[3] << "motorSpeedRight:" << value[4];
            plot->ShowPlot_WaveForm(plot->pPlot1, value);
        }
        plot->setAutoX(plot->pPlot1,group_index[selectedIndex].second + 10);


        QStringList lineNames;
        lineNames << "currentDistance" << "lineSeparation" << "rudderAngle" << "motorSpeedLeft" << "motorSpeedRight";
        plot->setCurvesName(plot->pPlot1,lineNames);
    }
}

// 发送2

// 保存
void MainWindow::on_pushButton_4_released()
{
    // QByteArray sendData = "2"; // 发送固定字符串 "0"

    // // 如发送成功，会返回发送的字节长度。失败，返回-1。
    // int a = mySerialPort->write(sendData);
    // // 发送字节计数并显示
    // if(a > 0)
    // {
    //     // 发送字节计数
    //     sendNum += a;
    //     // 状态栏显示计数值
    //     setNumOnLabel(lblSendNum, "S: ", sendNum);
    // }
    QString fileName = QCoreApplication::applicationDirPath() + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        // out << ui->textEdit->toPlainText();
        out << ui->txtRec->toPlainText();
        file.close();
        QMessageBox::information(this, "提示", "文件已保存为: " + fileName);
    }
    else
    {
        QMessageBox::warning(this, "警告", "无法保存文件");
    }

    // refrese files
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(1); // 修改列数为1
    model->setHeaderData(0, Qt::Horizontal, "名称");

    QDir directory(QCoreApplication::applicationDirPath());
    QStringList txtFiles = directory.entryList(QStringList() << "*.txt", QDir::Files);
    for (int i = 0; i < txtFiles.size(); ++i)
    {
        QList<QStandardItem *> row;
        row.append(new QStandardItem(txtFiles.at(i))); // 仅添加名称列
        model->appendRow(row);
    }

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    qDebug() << "当前目录下的txt文件导入表格完成";
}
void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    dataParser->test();
    // bug:会连续触发
    if (index.isValid())
    {
        // 获取点击行的序号（第一列的值）
        QString number = QString::number(index.row() + 1);
        qDebug() << "点击了第" << number << "行";

        // 记录选中的文件序号
        file_selected = number.toInt() - 1;

        // 获取点击行的名称（第二列的值）
        QString fileName = index.sibling(index.row(), 0).data().toString();
        qDebug() << "文件名称：" << fileName;

        // 构造文件路径
        QString filePath = QCoreApplication::applicationDirPath() + "/" + fileName;

        // 读取文件内容并显示在 textEdit 控件上
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            ui->txtRec->setPlainText(in.readAll());
            file.close();
            // this->setWindowTitle(fileName); // 更新窗口标题为文件名
        }
        else
        {
            QMessageBox::warning(this, "警告", "无法打开文件: " + fileName);
        }
    }
}

void MainWindow::onKey1Pressed()
{
    QByteArray sendData = "1"; // 发送固定字符串 "2"

    qDebug() << "数字键1按下";
    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    int a = mySerialPort->write(sendData);
    // 发送字节计数并显示
    if (a > 0)
    {
        // 发送字节计数
        sendNum += a;
        // 状态栏显示计数值
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }

    qDebug() << "executed!";
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_1)
        {
            onKey1Pressed();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_0)
        {
            onKey0Pressed();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_2)
        {
            onKey2Pressed();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_3)
        {
            onKey3Pressed();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_4)
        {
            onKey4Pressed();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_5)
        {
            onKey5Pressed();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_6)
        {
            onKey6Pressed();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_7)
        {
            onKey7Pressed();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_8)
        {
            onKey8Pressed();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Delete)
        {
            // 删除键响应
            if (file_selected != NO_FILE_SELECTED)
            {
                // 获取当前文件名
                QAbstractItemModel *model = ui->tableView->model();
                QModelIndex index = model->index(file_selected, 0);
                QString fileName = model->data(index).toString();
                QDir dir(QCoreApplication::applicationDirPath());
                if (dir.remove(fileName))
                {
                    qDebug() << "文件删除成功:" << fileName;
                }
                else
                {
                    qDebug() << "文件删除失败:" << fileName;
                }
                // 刷新文件列表
                QStandardItemModel *newModel = new QStandardItemModel(this);
                newModel->setColumnCount(1);
                newModel->setHeaderData(0, Qt::Horizontal, "名称");
                QStringList txtFiles = dir.entryList(QStringList() << "*.txt", QDir::Files);
                for (int i = 0; i < txtFiles.size(); ++i)
                {
                    QList<QStandardItem *> row;
                    row.append(new QStandardItem(txtFiles.at(i)));
                    newModel->appendRow(row);
                }
                ui->tableView->setModel(newModel);
                ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
                file_selected = NO_FILE_SELECTED;
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onKey0Pressed()
{
    QByteArray sendData = "0"; // 发送固定字符串 "0"

    qDebug() << "数字键0按下";
    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    int a = mySerialPort->write(sendData);
    // 发送字节计数并显示
    if (a > 0)
    {
        // 发送字节计数
        sendNum += a;
        // 状态栏显示计数值
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }

    qDebug() << "executed!";
}
void MainWindow::onKey2Pressed()
{
    QByteArray sendData = "2"; // 发送固定字符串 "2"

    qDebug() << "数字键2按下";
    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    int a = mySerialPort->write(sendData);
    // 发送字节计数并显示
    if (a > 0)
    {
        // 发送字节计数
        sendNum += a;
        // 状态栏显示计数值
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }

    qDebug() << "executed!";
}

void MainWindow::onKey3Pressed()
{
    QByteArray sendData = "3"; // 发送固定字符串 "3"

    qDebug() << "数字键3按下";
    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    int a = mySerialPort->write(sendData);
    // 发送字节计数并显示
    if (a > 0)
    {
        // 发送字节计数
        sendNum += a;
        // 状态栏显示计数值
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }

    qDebug() << "executed!";
}

void MainWindow::onKey4Pressed()
{
    QByteArray sendData = "4"; // 发送固定字符串 "4"

    qDebug() << "数字键4按下";
    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    int a = mySerialPort->write(sendData);
    // 发送字节计数并显示
    if (a > 0)
    {
        // 发送字节计数
        sendNum += a;
        // 状态栏显示计数值
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }

    qDebug() << "executed!";
}
void MainWindow::onKey5Pressed()
{
    QByteArray sendData = "5"; // 发送固定字符串 "5"

    qDebug() << "数字键5按下";
    int a = mySerialPort->write(sendData);
    if (a > 0)
    {
        sendNum += a;
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }
    qDebug() << "executed!";
}

void MainWindow::onKey6Pressed()
{
    QByteArray sendData = "6"; // 发送固定字符串 "6"

    qDebug() << "数字键6按下";
    int a = mySerialPort->write(sendData);
    if (a > 0)
    {
        sendNum += a;
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }
    qDebug() << "executed!";
}

void MainWindow::onKey7Pressed()
{
    QByteArray sendData = "7"; // 发送固定字符串 "7"

    qDebug() << "数字键7按下";
    int a = mySerialPort->write(sendData);
    if (a > 0)
    {
        sendNum += a;
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }
    qDebug() << "executed!";
}
void MainWindow::onKey8Pressed()
{
    QByteArray sendData = "8"; // 发送固定字符串 "8"
    qDebug() << "数字键8按下";
    int a = mySerialPort->write(sendData);
    if (a > 0)
    {
        sendNum += a;
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }
}
/*
收起与展开*/
void MainWindow::on_inandoutButton_released()
{
    // 获取按钮的名称
    QString buttonText = ui->inandoutButton->text();
    if (buttonText == "<")
    {
        ui->inandoutButton->setText(">");
        // 展开区域
        ui->frame->setVisible(true);
    }
    else
    {
        ui->inandoutButton->setText("<");
        // 收起区域
        ui->frame->setVisible(false);
    }
}

void MainWindow::on_TestButton_released()
{
    if (plot)
    {
        delete plot;
        plot = nullptr;
    }

    // 创建一个新的 Plot 对象
    plot = new Plot;

    // 显示新的波形绘图窗口
    plot->show();
    // 修改plot的标题

    int rowIndex = file_selected;
    QString secondColumnData = ui->tableView->model()->index(rowIndex, 0).data().toString();
    qDebug() << "Second column content:" << secondColumnData;
    // QString plotTitle = "波形显示";
    plot->setWindowTitle(secondColumnData);
    float value[20] = {1, 2, 3};
    for (int i = 0; i < 100; i++)
    {
        // 将 j 的类型改为 int，并修正迭代变量
        for (int j = 0; j < 3; j++)
            value[j] = j + 0.5;
        plot->ShowPlot_WaveForm(plot->pPlot1, value);
    }
}
