#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "test.h"

QStringList CurveLineNames;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Qt Serial Debugger");
    ui->txtRec->setLineWrapMode(QPlainTextEdit::NoWrap);
    CurveLineNames << "currentDistance" << "lineSeparation" << "rudderAngle" << "motorSpeedLeft" << "motorSpeedRight" << "phaseFlag" << "speed" << "goDestSpeed" << "originBearing" << "currentBearing" << "currentYaw" << "bearingError" << "yawCurrentBearing" << "kp_angle" << "minYawDeviation" << "maxYawDeviation" << "yawDeviation" << "imuYaw" << "ddmYaw" << "gpsYaw";

    // 查找当前目录下的txt文件并导入表格
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(1); // 修改列数为1
    model->setHeaderData(0, Qt::Horizontal, "名称");

    QStringList txtFiles = FileHelper::findAllTxtFiles(RAW);
    qDebug() << "txtFiles:" << txtFiles;
    for (int i = 0; i < txtFiles.size(); ++i)
    {
        QList<QStandardItem *> row;
        row.append(new QStandardItem(txtFiles.at(i))); // 仅添加名称列
        model->appendRow(row);
    }

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
    plot = new Plot(logData, group_index);

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
    /* 帧过滤部分代码 */

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
}

void MainWindow::serialPortRead_Slot()
{
    QByteArray recBuf;
    recBuf = mySerialPort->readAll();

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
    plot = new Plot(logData, group_index);

    // 显示新的波形绘图窗口
    plot->show();
    // 修改plot的标题

    int rowIndex = file_selected;
    QString secondColumnData = ui->tableView->model()->index(rowIndex, 0).data().toString();
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
#define testdd
// 测试1
void MainWindow::on_pushButton_3_released()
{
    int GroupCount = 0;
    int group_count = 0;
    int idx_index = 0; // 每组的起始点
    memset(logData, 0, sizeof(logData));
    memset(group_index, 0, sizeof(group_index));

    QString plainText = ui->txtRec->toPlainText();

    dataParser->parseData(plainText, group_index, logData, GroupCount, group_count, idx_index);

    QStringList groupOptions;
    for (int i = 0; i <= group_count; ++i)
    {
        groupOptions << QString("第%1组").arg(i + 1);
    }
    if (groupOptions.isEmpty())
    {
        PlotError error(PlotError::KnownError, "没有找到有效数据，请检查数据格式或者数据解析方式是否正确");
        PlotError::showErrorDialog(this, error);
    }
    else
    {
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
            plot = new Plot(logData, group_index);
            ;

            // 显示新的波形绘图窗口
            plot->showMaximized();
            // 修改plot的标题
            plot->setSelectedGroup(selectedIndex);
            int rowIndex = file_selected;
            QString secondColumnData = ui->tableView->model()->index(rowIndex, 0).data().toString();
            // QString plotTitle = "波形显示";
            plot->setWindowTitle(secondColumnData);
            int flag[3] = {0};
            float value[20] = {0};
            float Kp[3] = {0};
            float Ki[3] = {0};
            float Kd[3] = {0};
            float Ki_limit[3] = {0};
            for (int j = 1; j <= group_index[selectedIndex].second; j++)
            {
                value[0] = logData[group_index[selectedIndex].first + j].currentDistance;
                value[2] = logData[group_index[selectedIndex].first + j].rudderAngle;
                value[3] = logData[group_index[selectedIndex].first + j].motorSpeedLeft;
                value[4] = logData[group_index[selectedIndex].first + j].motorSpeedRight;
                value[5] = logData[group_index[selectedIndex].first + j].phaseFlag;
                value[7] = logData[group_index[selectedIndex].first + j].goDestSpeed;

                // 未分析
                value[8] = logData[group_index[selectedIndex].first + j].firstPhaseCount;
                value[9] = logData[group_index[selectedIndex].first + j].originBearing;   // 原始航向
                value[10] = logData[group_index[selectedIndex].first + j].currentBearing; // 当前航向
                value[11] = logData[group_index[selectedIndex].first + j].currentYaw;
                value[12] = logData[group_index[selectedIndex].first + j].bearingError;
                value[13] = logData[group_index[selectedIndex].first + j].yawCurrentBearing;
                value[14] = logData[group_index[selectedIndex].first + j].kp_angle;
                value[15] = logData[group_index[selectedIndex].first + j].minYawDeviation;
                value[16] = logData[group_index[selectedIndex].first + j].maxYawDeviation;
                value[17] = logData[group_index[selectedIndex].first + j].yawDeviation; // 航向偏差
                value[18] = logData[group_index[selectedIndex].first + j].imuYaw;
                value[19] = logData[group_index[selectedIndex].first + j].timestamp;
                // value[20] = logData[group_index[selectedIndex].first + j].gpsYaw;

                // 因为y值变化幅度小要单独显示在右侧轴的曲线
                value[1] = logData[group_index[selectedIndex].first + j].lineSeparation; // 航线间距
                value[6] = logData[group_index[selectedIndex].first + j].speed;          // 速度 米/秒

                if (flag[0] == 0)
                {
                    if (logData[group_index[selectedIndex].first + j].phaseFlag == 1)
                    {
                        Kp[0] = logData[group_index[selectedIndex].first + j].kp_yaw_first;
                        Ki[0] = logData[group_index[selectedIndex].first + j].ki_yaw_first;
                        Kd[0] = logData[group_index[selectedIndex].first + j].kd_yaw_first;
                        Ki_limit[0] = logData[group_index[selectedIndex].first + j].integralLimit_yaw_first;
                        flag[0] = 1;
                    }
                }
                else if (flag[1] == 0)
                {
                    if (logData[group_index[selectedIndex].first + j].phaseFlag == 2)
                    {
                        Kp[1] = logData[group_index[selectedIndex].first + j].kp_pos;
                        Ki[1] = logData[group_index[selectedIndex].first + j].ki_pos;
                        Kd[1] = logData[group_index[selectedIndex].first + j].kd_pos;
                        Ki_limit[1] = logData[group_index[selectedIndex].first + j].integralLimit_pos;
                        flag[1] = 1;
                    }
                }
                else if (flag[2] == 0)
                {
                    if (logData[group_index[selectedIndex].first + j].phaseFlag == 3)
                    {
                        Kp[2] = logData[group_index[selectedIndex].first + j].kp_yaw_third;
                        Ki[2] = logData[group_index[selectedIndex].first + j].ki_yaw_third;
                        Kd[2] = logData[group_index[selectedIndex].first + j].kd_yaw_third;
                        Ki_limit[2] = logData[group_index[selectedIndex].first + j].integralLimit_yaw_third;
                        flag[2] = 1;
                    }
                }
                // 输出一下
                // qDebug() << "currentDistance:" << value[0] << "lineSeparation:" << value[1] << "rudderAngle:" << value[2] << "speed" << value[6];
                plot->ShowPlot_WaveForm(plot->pPlot1, value);
            }
            for (int i = 0; i < 3; i++)
                plot->setPid(i, Kp[i], Ki[i], Kd[i], Ki_limit[i]);
            plot->setAutoX(plot->pPlot1, group_index[selectedIndex].second + 10);

            // 主动隐藏阶段曲线
            plot->hideCurve(5);
            plot->hideCurve(0); // 距离线
            plot->hideCurve(6); // 速度线
            plot->hideCurve(7); //
            for (int i = 8; i < 20; i++)
            {
                plot->hideCurve(i);
            }
            QStringList templist;
            DataParser parsefp;
            for (int i = 0; i < CurveLineNames.size(); i++)
            {
                templist << parsefp.parseData(CurveLineNames.at(i));
            }

            plot->setCurvesName(templist);

            // 阶段区分
            plot->stageDistinguish();
        }
    }
}

// 发送2

// 保存
void MainWindow::on_pushButton_4_released()
{
    QString fileName = FileHelper::saveTxtFile(ui->txtRec->toPlainText(), RAW);
    if (!fileName.isEmpty())
    {
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

    FilesReflash();
    qDebug() << "当前目录下的txt文件导入表格完成";
}
void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    // test
    // int GroupCount = 0;
    // int group_count = 0;
    // int idx_index = 0; // 每组的起始点
    // QString plainText = ui->txtRec->toPlainText();

    // dataParser->parseData(plainText, group_index, logData, GroupCount, group_count, idx_index);
    // test

    // bug:会连续触发
    if (index.isValid())
    {
        // 获取点击行的序号（第一列的值）
        QString number = QString::number(index.row() + 1);

        // 记录选中的文件序号
        file_selected = number.toInt() - 1;

        // 获取点击行的名称（第二列的值）
        QString fileName = index.sibling(index.row(), 0).data().toString();
        QString readtxt;
        if (FileHelper::readtxtFile(fileName, RAW, readtxt))
        {
            ui->txtRec->setPlainText(readtxt);
        }
        else
        {
            QMessageBox::warning(this, "警告", "无法打开文件: " + fileName);
        }
    }
}

void MainWindow::scrollToString(const QString &targetString)
{
    // 获取全部文本
    QString text = ui->txtRec->toPlainText();
    int pos = text.indexOf(targetString);
    if (pos >= 0)
    {
        // 创建光标并定位
        QTextCursor cursor = ui->txtRec->textCursor();
        cursor.setPosition(pos);

        // 选中整行
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

        // 设置高亮
        QTextEdit::ExtraSelection highlight;
        highlight.cursor = cursor;
        highlight.format.setBackground(Qt::yellow); // 高亮颜色

        QList<QTextEdit::ExtraSelection> extras;
        extras << highlight;
        ui->txtRec->setExtraSelections(extras);

        // 滚动到光标处
        ui->txtRec->setTextCursor(cursor);

        // 强制将水平滚动条移到最左侧（行首）
        ui->txtRec->horizontalScrollBar()->setValue(ui->txtRec->horizontalScrollBar()->minimum());
    }
    else
    {
        // 清除高亮
        ui->txtRec->setExtraSelections({});
        QMessageBox::information(this, "查找结果", "未找到指定字符串: " + targetString);
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
void MainWindow::FilesReflash(void)
{
    // 刷新文件列表
    QStandardItemModel *newModel = new QStandardItemModel(this);
    newModel->setColumnCount(1);
    newModel->setHeaderData(0, Qt::Horizontal, "名称");
    QStringList txtFiles = FileHelper::findAllTxtFiles(RAW);
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
                if (FileHelper::removeTxtFile(fileName, RAW))
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
                FilesReflash();
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
    DataParser d;
    // 属性项名称
    QStringList headers;
    headers << "timestamp"
            << "currentMode"
            << "phaseFlag"
            << "goDestSpeed"
            << "firstPhaseCount"
            << "originBearing"
            << "currentBearing"
            << "currentYaw"
            << "currentDistance"
            << "lineSeparation"
            << "bearingError"
            << "yawCurrentBearing"
            << "rudderAngle"
            << "motorSpeedLeft"
            << "motorSpeedRight"
            << "kp_yaw_first"
            << "ki_yaw_first"
            << "kd_yaw_first"
            << "integralLimit_yaw_first"
            << "kp_pos"
            << "ki_pos"
            << "kd_pos"
            << "integralLimit_pos"
            << "kp_yaw_third"
            << "ki_yaw_third"
            << "kd_yaw_third"
            << "integralLimit_yaw_third"
            << "latitude"
            << "longitude"
            << "speed"
            << "kp_angle"
            << "minYawDeviation"
            << "maxYawDeviation"
            << "yawDeviation"
            << "imuYaw"
            << "ddmYaw"
            << "gpsYaw";
    for (int i = 0; i < headers.size(); i++)
    {
        QString temp = d.parseData(headers.at(i));
        headers[i] = temp;
    }
    int rowCount = 5; // 示例数据行数
    int colCount = headers.size();

    QStandardItemModel *model = new QStandardItemModel(rowCount, colCount, this);
    model->setHorizontalHeaderLabels(headers);

    // 填充示例数据
    for (int row = 0; row < rowCount; ++row)
    {
        for (int col = 0; col < colCount; ++col)
        {
            QStandardItem *item = new QStandardItem(QString("R%1C%2").arg(row + 1).arg(col + 1));
            model->setItem(row, col, item);
        }
    }
    ui->structed_table->setModel(model);
    // ui->structed_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->structed_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::on_pushButton_2_released()
{
    qDebug() << "test";
    scrollToString("454247: GODEST: 1 2 80 1  178.955  177.908  178.360");
}
