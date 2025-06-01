#include "plot.h"
#include "ui_plot.h"
#include "new_data_parser.h"
#include <stdio.h>
#include <QFontMetrics>
#include <QStandardItemModel>

#include "mainwindow.h"
QStringList CurveLineNamesInChinese;

int defaultLegendIndex[5] = {5, 6, 9, 10, 11};

Plot::Plot(GODEST_log_data_t *logDataPtr, std::pair<int, int> group_index[100], QWidget *parent)
    : QMainWindow(parent),
      logDataPtr(logDataPtr),
      ui(new Ui::Plot)
{
    // 将 group_index 拷贝到成员变量
    this->plot_group_index = group_index;

    ui->setupUi(this);
    setWindowTitle("Plot");

    // 给widget绘图控件，设置个别名，方便书写
    pPlot1 = ui->winPlot;
    // 状态栏指针
    sBar = statusBar();

    // 初始化图表1
    QPlot_init(pPlot1);
    // 绘图图表的设置控件初始化，主要用于关联控件的信号槽
    QPlot_widget_init();

    // 创建定时器，用于定时生成曲线坐标点数据
    timer = new QTimer(this);
    timer->setInterval(10);
    connect(timer, SIGNAL(timeout()), this, SLOT(TimeData_Update()));
    // timer->start(10);

    // 关联控件初始化
    // ui->txtPointOriginX->setEnabled(false);
    // 图表重绘后，刷新原点坐标和范围
    connect(pPlot1, SIGNAL(afterReplot()), this, SLOT(repPlotCoordinate()));
}

Plot::~Plot()
{
    qDebug() << "~Plot";

    // 停止定时器
    if (timer && timer->isActive())
    {
        timer->stop();
    }
    delete timer;
    timer = nullptr;

    // 清空绘图数据
    if (pPlot1)
    {
        for (int i = 0; i < 20; i++)
        {
            if (pCurve[i])
            {
                pCurve[i]->data().data()->clear();
            }
        }
        pPlot1->clearGraphs();
        pPlot1->replot(QCustomPlot::rpQueuedReplot);
    }

    // 初始化计数器
    cnt = 0;
    // 释放 UI 资源
    delete ui;
}
void Plot::addFrameToWinPlot()
{
    QString temp;
    QString temp2;
    QStringList templist; // 对齐使用
    QString info;
    //bool IsFullyDisplay = true; // 修正不同屏幕下的显示问题
    int addnum = 0;
    int temp_count3 = 1;
    int temp_count4 = 3;
    int frameWidth = 350;
    int frameHeight = 300;
    if (!frame)
    {
        frame = new QFrame(ui->winPlot);
        frame->setAttribute(Qt::WA_TranslucentBackground, true);
        frame->setWindowFlags(Qt::FramelessWindowHint);
        frame->setStyleSheet("background:transparent; border:none;");

        // 创建网格布局
        QGridLayout *gridLayout = new QGridLayout(frame);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setSpacing(2);

        // 动态添加若干 QLabel 到 frame，4列2行
        QStringList testTexts = {
            "测试文字1", "测试文字2", "测试文字3", "测试文字4",
            "测试文字5", "测试文字6", "测试文字7", "测试文字8",
            "测试文字9", "测试文字10", "测试文字11", "测试文字12",
            "测试文字13", "测试文字14", "测试文字15", "测试文字16",
            "测试文字17", "测试文字18", "测试文字19", "测试文字20",
            "测试文字21", "测试文字22", "测试文字23", "测试文字24",
            "测试文字25", "测试文字26", "测试文字27", "测试文字28",
            "测试文字29", "测试文字30", "", "",
            "测试文字33", "测试文字34", "", "",
            "测试文字37", "测试文字38", "", "",
            "测试文字41", "测试文字42", "", "",
            "测试文字45", "测试文字46"};
        int temp_count = 0;
        int temp_count2 = 2;
        for (int i = 0; i < CurveLineNamesInChinese.size(); ++i)
        {
            if (i < 12)
            {
                testTexts[temp_count] = CurveLineNamesInChinese[i];
                temp_count += 4;
            }
            // else{
            //     testTexts[temp_count2] = CurveLineNamesInChinese[i];
            //     temp_count2 += 4;
            // }
        }
        testTexts[2] = CurveLineNamesInChinese.at(12);
        testTexts[6] = CurveLineNamesInChinese.at(13);
        testTexts[10] = CurveLineNamesInChinese.at(14);
        testTexts[14] = CurveLineNamesInChinese.at(15);
        testTexts[18] = CurveLineNamesInChinese.at(16);
        testTexts[22] = CurveLineNamesInChinese.at(17);
        testTexts[26] = CurveLineNamesInChinese.at(18);
        testTexts[30] = CurveLineNamesInChinese.at(19);

        for (int i = 0; i < testTexts.size(); ++i)
        {
            QLabel *label = new QLabel(testTexts[i], frame);
            label->setStyleSheet("color:black; font: 10pt 'Consolas'; background:transparent; border:none;");
            int row = i / 4;
            int col = i % 4;
            gridLayout->addWidget(label, row, col);
            labels.append(label);
        }

        // 设置 frame 的大小和位置（顶部居中，紧贴winPlot顶部）

        frame->setFixedSize(frameWidth, frameHeight);
        frame->move((ui->winPlot->width() - frameWidth) / 2, 0); // 顶部居中

        // 让 frame 始终在最上层
        frame->raise();
        frame->show();
    }
    else
    {

        for (int i = 0; i < 20; ++i)
        {
            QCPGraph *graph = pPlot1->graph(i);
            if (!graph)
                continue;
            // 解析出数字部分，包括符号
            QRegularExpression re(R"([-+]?\d*\.?\d+)");
            QRegularExpressionMatch match = re.match(graph->name());
            if (match.hasMatch())
            {
                temp = match.captured(0);
            }
            else
            {
                temp = "";
            }
            templist << temp;
        }
    }
    // 如果已存在，只需修改 label 内容
    QStringList testTexts = {
        "测试文字1", "测试文字2", "测试文字3", "测试文字4",
        "测试文字5", "测试文字6", "测试文字7", "测试文字8",
        "测试文字9", "测试文字10", "测试文字11", "测试文字12",
        "测试文字13", "测试文字14", "测试文字15", "测试文字16",
        "测试文字17", "测试文字18", "测试文字19", "测试文字20",
        "测试文字21", "测试文字22", "测试文字23", "测试文字24",
        "测试文字25", "测试文字26", "测试文字27", "测试文字28",
        "测试文字29", "测试文字30", "测试文字31", "测试文字32",
        "测试文字33", "测试文字34", "测试文字35", "测试文字36",
        "测试文字37", "测试文字38", "测试文字39", "测试文字40",
        "测试文字41", "测试文字42", "测试文字43", "测试文字44",
        "测试文字45", "测试文字46"};
    for (int i = 0; i < templist.size(); ++i)
    {
        if (i < 12)
        {
            if (i == 0)
                labels[temp_count3]->setText(QString("%1     ").arg(templist[i].toDouble(), 8, 'f', 0));
            else
                labels[temp_count3]->setText(QString("%1     ").arg(templist[i].toDouble(), 8, 'f', 2));
            temp_count3 += 4;
        }
        else if (i >= 12)
        {
            labels[temp_count4]->setText(QString("%1").arg(templist[i].toDouble(), 8, 'f', 2));
            temp_count4 += 4;
        }
        // if (IsFullyDisplay)
        // {
        //     QFontMetrics fm(labels[i]->font());
        //     int textWidth = fm.horizontalAdvance(labels[i]->text());
        //     if (textWidth > labels[i]->width())
        //     {
        //         qDebug() << "Label " << i << " text is not fully displayed after update.";
        //         frameWidth += 100;
        //         IsFullyDisplay = false;
        //     }
        // }
    }
    // for (int i = 0; i < templist.size(); i++)
    // {
    //     if (i >= 12)
    //     {
    //         labels[temp_count4]->setText(QString("%1").arg(templist[i].toDouble(), 8, 'f', 2));
    //         temp_count4 += 4;
    //     }
    // }

    // 重新调整位置和显示

    frame->setFixedSize(frameWidth, frameHeight);
    frame->move((ui->winPlot->width() - frameWidth) / 2, 0);
    frame->raise();
    frame->show();
}

// 6. 如需后续访问这些 label，可将 labels 保存为成员变量

// 绘图图表初始化
void Plot::QPlot_init(QCustomPlot *customPlot)
{
    qDebug() << "QPlot_init";
    // addFrameToWinPlot();
    //  添加曲线名称、设置图例的文本
    QStringList lineNames;
    lineNames << "bbq" << "波形2" << "波形3" << "波形4" << "波形5" << "波形6" << "速度" << "波形8" << "波形9" << "波形10"
              << "波形11" << "波形12" << "波形13" << "波形14" << "波形15" << "波形16" << "波形17" << "波形18" << "波形19" << "波形20";
    // 曲线初始颜色
    QColor initColor[20] = {QColor(243, 194, 70), QColor(162, 0, 124), QColor(172, 172, 172), QColor(0, 178, 191), QColor(197, 124, 172),
                            QColor(0, 140, 94), QColor(178, 0, 31), QColor(91, 189, 43), QColor(0, 219, 219), QColor(241, 175, 0),
                            QColor(27, 79, 147), QColor(229, 70, 70), QColor(0, 146, 152), QColor(115, 136, 193), QColor(245, 168, 154),
                            QColor(152, 208, 185), QColor(223, 70, 41), QColor(175, 215, 136), QColor(157, 255, 255), QColor(0, 0, 0)}; // QColor(255,255,255)};//白色
    // 图表添加20条曲线，并设置初始颜色，和图例名称
    for (int i = 0; i < 20; i++)
    {
        pCurve[i] = customPlot->addGraph();
        pCurve[i]->setPen(QPen(QColor(initColor[i])));
        pCurve[i]->setName(lineNames.at(i));
        // 设置为平滑曲线
        pCurve[i]->setLineStyle(QCPGraph::lsLine);
        pCurve[i]->setScatterStyle(QCPScatterStyle::ssNone);
        pCurve[i]->setAdaptiveSampling(false); // 关闭自适应采样，保证平滑
        pCurve[i]->setAntialiased(true);       // 抗锯齿
    }

    // 属于右侧轴的曲线使用实心圆表示，作为区分。
    // curveSetScatterStyle(pPlot1, pCurve[1], 5);
    // curveSetScatterStyle(pPlot1, pCurve[6], 5);
    // 设置y轴2，与y轴共享x轴，添加曲线1到y轴2
    customPlot->yAxis2->setVisible(true);
    pCurve[6]->setValueAxis(customPlot->yAxis2);
    pCurve[12]->setValueAxis(customPlot->yAxis2);
    customPlot->yAxis2->setLabel("线距");
    // 保证yAxis2的0刻度与yAxis对齐，并随左轴拖动同步移动
    connect(customPlot->yAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged),
            [customPlot](const QCPRange &leftRange)
            {
                double leftSpan = leftRange.upper - leftRange.lower;
                double leftCenter = (leftRange.upper + leftRange.lower) / ZOOMSTANDARD;
                double newRightLower = leftCenter - leftSpan / ZOOMSTANDARD;
                double newRightUpper = leftCenter + leftSpan / ZOOMSTANDARD;
                customPlot->yAxis2->setRange(newRightLower, newRightUpper);
            });
    // 初始化时也对齐一次
    QCPRange leftRange = customPlot->yAxis->range();
    double leftSpan = leftRange.upper - leftRange.lower;
    double leftCenter = (leftRange.upper + leftRange.lower) / ZOOMSTANDARD;
    double newRightLower = leftCenter - leftSpan / ZOOMSTANDARD;
    double newRightUpper = leftCenter + leftSpan / ZOOMSTANDARD;
    customPlot->yAxis2->setRange(newRightLower, newRightUpper);

    QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);

    // 设置y轴2的主刻度间隔为1
    fixedTicker->setTickStep(1);
    fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
    customPlot->yAxis2->setTicker(fixedTicker);

    // 曲线设置粗细
    QPen pen;
    for (int i = 0; i < 13; i++)
    {
        if (i == 10 || i == 11)
            ;
        else
            continue;
        pen = pCurve[i]->pen();
        pen.setWidth(2);
        pCurve[i]->setPen(pen);
    }

    // 设置背景颜色
    customPlot->setBackground(QColor(255, 255, 255));
    // 设置背景选择框颜色
    ui->btnColourBack->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(QColor(255, 255, 255).name()));

    // 设置坐标轴名称
    customPlot->xAxis->setLabel("TimeStamp");
    customPlot->yAxis->setLabel("Y");

    // 设置x,y坐标轴显示范围
    //    pointCountX = ui->txtPointCountX->text().toUInt();
    //    pointCountY = ui->txtPointCountY->text().toUInt();
    customPlot->xAxis->setRange(0, 1000);
    customPlot->yAxis->setRange(400 / 2 * -1, 400 / 2);

    customPlot->xAxis->ticker()->setTickCount(ui->txtMainScaleNumX->text().toUInt()); // 11个主刻度
    customPlot->yAxis->ticker()->setTickCount(ui->txtMainScaleNumY->text().toUInt()); // 11个主刻度
    customPlot->xAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);  // 可读性优于设置
    customPlot->yAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);  // 可读性优于设置

    // 图例显示
    showDashboard(customPlot);

    // 允许用户用鼠标拖动轴范围，以鼠标为中心滚轮缩放，点击选择图形:
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    // 设置鼠标滚轮的缩放倍率，如果不设置默认为0.85，大于1反方向缩放
    // customPlot->axisRect()->setRangeZoomFactor(0.5);
    // 设置鼠标滚轮缩放的轴方向，仅设置垂直轴。垂直轴和水平轴全选使用：Qt::Vertical | Qt::Horizontal
    customPlot->axisRect()->setRangeZoom(Qt::Vertical);
    // customPlot->axisRect()->setRangeZoom(Qt::Horizontal);
    /*
    游标的设置
    */

    // 生成每条曲线的tracer
    for (int i = 0; i < customPlot->graphCount(); ++i)
    {
        QCPItemTracer *t = new QCPItemTracer(customPlot);
        t->setGraph(customPlot->graph(i));
        t->setStyle(QCPItemTracer::tsCircle);
        t->setPen(QPen(Qt::red));
        t->setBrush(QBrush(Qt::red));
        t->setSize(7);
        t->setVisible(false); // 初始隐藏
        tracers.append(t);
    }

    // 生成竖线
    vLine = new QCPItemStraightLine(customPlot);
    vLine->setPen(QPen(Qt::blue, 1, Qt::DashLine));
    vLine->point1->setCoords(0, 0);
    vLine->point2->setCoords(0, 1);

    // 信号-槽连接语句
    connect(customPlot, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(mouseMove2(QMouseEvent *)));
    connect(customPlot, SIGNAL(mousePress(QMouseEvent *)), this, SLOT(onPlotClicked(QMouseEvent *))); // 新增：鼠标点击事件信号连接
}

/*
    功能：设置曲线名称，说明作用
    参数：
        lineNames：曲线名称列表
*/
void Plot::setCurvesName(QStringList lineNames)
{
    QStringList DefaultlineNames; // 设置图例的文本
    DefaultlineNames << "bbq" << "波形2" << "波形3" << "波形4" << "波形5" << "波形6" << "速度" << "速度" << "速度" << "速度"
                     << "波形11" << "波形12" << "波形13" << "波形14" << "波形15" << "波形16" << "波形17" << "波形18" << "波形19" << "波形20";

    for (int i = 0; i < lineNames.size() && i < 20; ++i)
    {
        DefaultlineNames[i] = lineNames[i];
    }
    lineNames = DefaultlineNames;

    // 图表添加20条曲线，并设置初始颜色，和图例名称
    for (int i = 0; i < 20; i++)
    {
        pCurve[i]->setName(lineNames.at(i));
    }
    // 侧栏名称修改

    ui->chkVisibleCurve1->setText(lineNames.at(0));
    ui->chkVisibleCurve2->setText(lineNames.at(1));
    ui->chkVisibleCurve3->setText(lineNames.at(2));
    ui->chkVisibleCurve4->setText(lineNames.at(3));
    ui->chkVisibleCurve5->setText(lineNames.at(4));
    ui->chkVisibleCurve6->setText(lineNames.at(5));
    ui->chkVisibleCurve7->setText(lineNames.at(6));
    ui->chkVisibleCurve8->setText(lineNames.at(7));
    ui->chkVisibleCurve9->setText(lineNames.at(8));
    ui->chkVisibleCurve10->setText(lineNames.at(9));
    ui->chkVisibleCurve11->setText(lineNames.at(10));
    ui->chkVisibleCurve12->setText(lineNames.at(11));
    ui->chkVisibleCurve13->setText(lineNames.at(12));
    ui->chkVisibleCurve14->setText(lineNames.at(13));
    ui->chkVisibleCurve15->setText(lineNames.at(14));
    ui->chkVisibleCurve16->setText(lineNames.at(15));
    ui->chkVisibleCurve17->setText(lineNames.at(16));
    ui->chkVisibleCurve18->setText(lineNames.at(17));
    ui->chkVisibleCurve19->setText(lineNames.at(18));
    ui->chkVisibleCurve20->setText(lineNames.at(19));
}

void Plot::setCurveslegendName(QStringList lineNames)
{
    QStringList DefaultlineNames; // 设置图例的文本
    DefaultlineNames << "bbq" << "波形2" << "波形3" << "波形4" << "波形5" << "波形6" << "速度" << "速度" << "速度" << "速度"
                     << "波形11" << "波形12" << "波形13" << "波形14" << "波形15" << "波形16" << "波形17" << "波形18" << "波形19" << "波形20";

    for (int i = 0; i < lineNames.size() && i < 20; ++i)
    {
        DefaultlineNames[i] = lineNames[i];
    }
    lineNames = DefaultlineNames;

    // 图表添加20条曲线，并设置初始颜色，和图例名称
    for (int i = 0; i < 20; i++)
    {
        pCurve[i]->setName(lineNames.at(i));
    }
}

void Plot::addCurvesName(QStringList addlineNames)
{
    // 获取侧栏当前值
    QStringList CurrenlineNames;
    CurrenlineNames << ui->chkVisibleCurve1->text() << ui->chkVisibleCurve2->text() << ui->chkVisibleCurve3->text() << ui->chkVisibleCurve4->text() << ui->chkVisibleCurve5->text();

    // 遍历 CurrenlineNames 并追加 addlineNames 对应位置的文本
    for (int i = 0; i < CurrenlineNames.size() && i < addlineNames.size(); ++i)
    {
        CurrenlineNames[i].append(addlineNames[i]);
    }

    // 侧栏名称修改
    ui->chkVisibleCurve1->setText(CurrenlineNames.at(0));
    ui->chkVisibleCurve2->setText(CurrenlineNames.at(1));
    ui->chkVisibleCurve3->setText(CurrenlineNames.at(2));
    ui->chkVisibleCurve4->setText(CurrenlineNames.at(3));
    ui->chkVisibleCurve5->setText(CurrenlineNames.at(4));
}

// 绘图图表的设置控件初始化，主要用于关联控件的信号槽
void Plot::QPlot_widget_init(void)
{
    // 获取控件指针数组，方便设置时编码书写
    pChkVisibleCurve[0] = ui->chkVisibleCurve1;
    pBtnColourCurve[0] = ui->btnColourCurve1;
    // pTxtValueCurve[0] = ui->txtValueCurve1;
    pRdoBoldCurve[0] = ui->rdoBoldCurve1;
    pChkVisibleCurve[1] = ui->chkVisibleCurve2;
    pBtnColourCurve[1] = ui->btnColourCurve2;
    // pTxtValueCurve[1] = ui->txtValueCurve2;
    pRdoBoldCurve[1] = ui->rdoBoldCurve2;
    pChkVisibleCurve[2] = ui->chkVisibleCurve3;
    pBtnColourCurve[2] = ui->btnColourCurve3;
    // pTxtValueCurve[2] = ui->txtValueCurve3;
    pRdoBoldCurve[2] = ui->rdoBoldCurve3;
    pChkVisibleCurve[3] = ui->chkVisibleCurve4;
    pBtnColourCurve[3] = ui->btnColourCurve4;
    // pTxtValueCurve[3] = ui->txtValueCurve4;
    pRdoBoldCurve[3] = ui->rdoBoldCurve4;
    pChkVisibleCurve[4] = ui->chkVisibleCurve5;
    pBtnColourCurve[4] = ui->btnColourCurve5;
    // pTxtValueCurve[4] = ui->txtValueCurve5;
    pRdoBoldCurve[4] = ui->rdoBoldCurve5;
    pChkVisibleCurve[5] = ui->chkVisibleCurve6;
    pBtnColourCurve[5] = ui->btnColourCurve6;
    // pTxtValueCurve[5] = ui->txtValueCurve6;
    pRdoBoldCurve[5] = ui->rdoBoldCurve6;
    pChkVisibleCurve[6] = ui->chkVisibleCurve7;
    pBtnColourCurve[6] = ui->btnColourCurve7;
    // pTxtValueCurve[6] = ui->txtValueCurve7;
    pRdoBoldCurve[6] = ui->rdoBoldCurve7;
    pChkVisibleCurve[7] = ui->chkVisibleCurve8;
    pBtnColourCurve[7] = ui->btnColourCurve8;
    // pTxtValueCurve[7] = ui->txtValueCurve8;
    pRdoBoldCurve[7] = ui->rdoBoldCurve8;
    pChkVisibleCurve[8] = ui->chkVisibleCurve9;
    pBtnColourCurve[8] = ui->btnColourCurve9;
    // pTxtValueCurve[8] = ui->txtValueCurve9;
    pRdoBoldCurve[8] = ui->rdoBoldCurve9;
    pChkVisibleCurve[9] = ui->chkVisibleCurve10;
    pBtnColourCurve[9] = ui->btnColourCurve10;
    // pTxtValueCurve[9] = ui->txtValueCurve10;
    pRdoBoldCurve[9] = ui->rdoBoldCurve10;
    pChkVisibleCurve[10] = ui->chkVisibleCurve11;
    pBtnColourCurve[10] = ui->btnColourCurve11;
    // pTxtValueCurve[10] = ui->txtValueCurve11;
    pRdoBoldCurve[10] = ui->rdoBoldCurve11;
    pChkVisibleCurve[11] = ui->chkVisibleCurve12;
    pBtnColourCurve[11] = ui->btnColourCurve12;
    // pTxtValueCurve[11] = ui->txtValueCurve12;
    pRdoBoldCurve[11] = ui->rdoBoldCurve12;
    pChkVisibleCurve[12] = ui->chkVisibleCurve13;
    pBtnColourCurve[12] = ui->btnColourCurve13;
    // pTxtValueCurve[12] = ui->txtValueCurve13;
    pRdoBoldCurve[12] = ui->rdoBoldCurve13;
    pChkVisibleCurve[13] = ui->chkVisibleCurve14;
    pBtnColourCurve[13] = ui->btnColourCurve14;
    // pTxtValueCurve[13] = ui->txtValueCurve14;
    pRdoBoldCurve[13] = ui->rdoBoldCurve14;
    pChkVisibleCurve[14] = ui->chkVisibleCurve15;
    pBtnColourCurve[14] = ui->btnColourCurve15;
    // pTxtValueCurve[14] = ui->txtValueCurve15;
    pRdoBoldCurve[14] = ui->rdoBoldCurve15;
    pChkVisibleCurve[15] = ui->chkVisibleCurve16;
    pBtnColourCurve[15] = ui->btnColourCurve16;
    // pTxtValueCurve[15] = ui->txtValueCurve16;
    pRdoBoldCurve[15] = ui->rdoBoldCurve16;
    pChkVisibleCurve[16] = ui->chkVisibleCurve17;
    pBtnColourCurve[16] = ui->btnColourCurve17;
    // pTxtValueCurve[16] = ui->txtValueCurve17;
    pRdoBoldCurve[16] = ui->rdoBoldCurve17;
    pChkVisibleCurve[17] = ui->chkVisibleCurve18;
    pBtnColourCurve[17] = ui->btnColourCurve18;
    // pTxtValueCurve[17] = ui->txtValueCurve18;
    pRdoBoldCurve[17] = ui->rdoBoldCurve18;
    pChkVisibleCurve[18] = ui->chkVisibleCurve19;
    pBtnColourCurve[18] = ui->btnColourCurve19;
    //  pTxtValueCurve[18] = ui->txtValueCurve19;
    pRdoBoldCurve[18] = ui->rdoBoldCurve19;
    pChkVisibleCurve[19] = ui->chkVisibleCurve20;
    pBtnColourCurve[19] = ui->btnColourCurve20;
    // pTxtValueCurve[19] = ui->txtValueCurve20;
    pRdoBoldCurve[19] = ui->rdoBoldCurve20;

    pCmbLineStyle[0] = ui->cmbLineStyle1;
    pCmbScatterStyle[0] = ui->cmbScatterStyle1;
    pCmbLineStyle[1] = ui->cmbLineStyle2;
    pCmbScatterStyle[1] = ui->cmbScatterStyle2;
    pCmbLineStyle[2] = ui->cmbLineStyle3;
    pCmbScatterStyle[2] = ui->cmbScatterStyle3;
    pCmbLineStyle[3] = ui->cmbLineStyle4;
    pCmbScatterStyle[3] = ui->cmbScatterStyle4;
    pCmbLineStyle[4] = ui->cmbLineStyle5;
    pCmbScatterStyle[4] = ui->cmbScatterStyle5;
    pCmbLineStyle[5] = ui->cmbLineStyle6;
    pCmbScatterStyle[5] = ui->cmbScatterStyle6;
    pCmbLineStyle[6] = ui->cmbLineStyle7;
    pCmbScatterStyle[6] = ui->cmbScatterStyle7;
    pCmbLineStyle[7] = ui->cmbLineStyle8;
    pCmbScatterStyle[7] = ui->cmbScatterStyle8;
    pCmbLineStyle[8] = ui->cmbLineStyle9;
    pCmbScatterStyle[8] = ui->cmbScatterStyle9;
    pCmbLineStyle[9] = ui->cmbLineStyle10;
    pCmbScatterStyle[9] = ui->cmbScatterStyle10;
    pCmbLineStyle[10] = ui->cmbLineStyle11;
    pCmbScatterStyle[10] = ui->cmbScatterStyle11;
    pCmbLineStyle[11] = ui->cmbLineStyle12;
    pCmbScatterStyle[11] = ui->cmbScatterStyle12;
    pCmbLineStyle[12] = ui->cmbLineStyle13;
    pCmbScatterStyle[12] = ui->cmbScatterStyle13;
    pCmbLineStyle[13] = ui->cmbLineStyle14;
    pCmbScatterStyle[13] = ui->cmbScatterStyle14;
    pCmbLineStyle[14] = ui->cmbLineStyle15;
    pCmbScatterStyle[14] = ui->cmbScatterStyle15;
    pCmbLineStyle[15] = ui->cmbLineStyle16;
    pCmbScatterStyle[15] = ui->cmbScatterStyle16;
    pCmbLineStyle[16] = ui->cmbLineStyle17;
    pCmbScatterStyle[16] = ui->cmbScatterStyle17;
    pCmbLineStyle[17] = ui->cmbLineStyle18;
    pCmbScatterStyle[17] = ui->cmbScatterStyle18;
    pCmbLineStyle[18] = ui->cmbLineStyle19;
    pCmbScatterStyle[18] = ui->cmbScatterStyle19;
    pCmbLineStyle[19] = ui->cmbLineStyle20;
    pCmbScatterStyle[19] = ui->cmbScatterStyle20;

    // 设置颜色选择框的初始背景颜色，与曲线同步颜色
    for (int i = 0; i < 20; i++)
    {
        pBtnColourCurve[i]->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(QColor(pCurve[i]->pen().color()).name()));
    }

    // 可见性选择框关联
    for (int i = 0; i < 20; i++)
    {
        connect(pChkVisibleCurve[i], &QCheckBox::clicked, [=]()
                { curveSetVisible(pPlot1, pCurve[i], pChkVisibleCurve[i]->checkState()); });
    }

    // 颜色选择框关联
    for (int i = 0; i < 20; i++)
    {
        connect(pBtnColourCurve[i], &QPushButton::clicked, [=]()
                { curveSetColor(pPlot1, pCurve[i], pBtnColourCurve[i]); });
    }

    // 加粗显示多选框关联。尽量别用，会导致CPU使用率升高
    for (int i = 0; i < 20; i++)
    {
        connect(pRdoBoldCurve[i], &QRadioButton::clicked, [=]()
                { curveSetBold(pPlot1, pCurve[i], pRdoBoldCurve[i]->isChecked()); });
    }

    // 曲线样式选择关联
    for (int i = 0; i < 20; i++)
    {
        connect(pCmbLineStyle[i], &QComboBox::currentTextChanged, [=]()
                { curveSetLineStyle(pPlot1, pCurve[i], pCmbLineStyle[i]->currentIndex()); });
    }

    // 散点样式选择关联
    for (int i = 0; i < 20; i++)
    {
        connect(pCmbScatterStyle[i], &QComboBox::currentTextChanged, [=]()
                { curveSetScatterStyle(pPlot1, pCurve[i], pCmbScatterStyle[i]->currentIndex() + 1); });
    }

    // QIcon ssCircleIcon (":/pic/ssCircle.png");
    // ui->cmbScatterStyle1->addItem(ssCircleIcon,"空心圆");
    for (int i = 0; i < 20; i++)
    {
        pCmbScatterStyle[i]->setIconSize(QSize(25, 17)); // 设置图片显示像素大小，不然会默认大小显示会模糊
    }
}

// 定时器溢出处理槽函数。用来生成曲线的坐标数据。
void Plot::TimeData_Update(void)
{
    // 生成坐标数据
    static float f;
    f += 0.01;
    // qDebug() << sin(f)*100;
    //  将坐标数据，传递给曲线
    ShowPlot_TimeDemo(pPlot1, sin(f) * 100);
}

// 曲线更新绘图，定时器绘图演示
void Plot::ShowPlot_TimeDemo(QCustomPlot *customPlot, double num)
{
    cnt++;
    // 给曲线添加数据
    for (int i = 0; i < 10; i++)
    {
        // pTxtValueCurve[i]->setText(QString::number(num - i * 10, 'g', 8)); // 显示曲线当前值
        pCurve[i]->addData(cnt, num - i * 10);
    }
    for (int i = 10; i < 20; i++)
    {
        //  pTxtValueCurve[i]->setText(QString::number(num + (i - 9) * 10, 'g', 8)); // 显示曲线当前值
        pCurve[i]->addData(cnt, num + (i - 9) * 10);
    }

    // 设置x坐标轴显示范围，使其自适应缩放x轴，x轴最大显示pointCountX个点。与chkTrackX复选框有关
    //    if (ui->chkTrackX->checkState())
    //    {
    //        // customPlot->xAxis->setRange((pCurve[0]->dataCount()>pointCountX)?(pCurve[0]->dataCount()-pointCountX):0, pCurve[0]->dataCount());
    //        setAutoTrackX(customPlot);
    //    }
    //    // 设置y坐标轴显示范围，使其自适应曲线缩放
    //    if (ui->chkAdjustY->checkState())
    //    {
    //        setAutoTrackY(customPlot);
    //    }

    // 更新绘图，这种方式在高填充下太浪费资源。有另一种方式rpQueuedReplot，可避免重复绘图。
    // 最好的方法还是将数据填充、和更新绘图分隔开。将更新绘图单独用定时器更新。例程数据量较少没用单独定时器更新，实际工程中建议大家加上。
    // customPlot->replot();
    customPlot->replot(QCustomPlot::rpQueuedReplot);

    static QTime time(QTime::currentTime());
    double key = time.elapsed() / 1000.0; // 开始到现在的时间，单位秒
    ////计算帧数
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key - lastFpsKey > 1) // 每1秒求一次平均值
    {
        // 状态栏显示帧数和数据总数
        ui->statusbar->showMessage(
            QString("%1 FPS, Total Data points: %2")
                .arg(frameCount / (key - lastFpsKey), 0, 'f', 0)
                .arg(customPlot->graph(0)->data()->size() + customPlot->graph(1)->data()->size()),
            0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

// 曲线更新绘图，波形数据绘图
void Plot::ShowPlot_WaveForm(QCustomPlot *customPlot, short value[])
{
    cnt++;
    // 给曲线添加数据
    for (int i = 0; i < 20; i++)
    {
        // QString strNum = QString::number(num,'g',8);// double类型
        // pTxtValueCurve[i]->setText(QString::number(value[i])); // 显示曲线当前值
        pCurve[i]->addData(cnt, value[i]); // 从原值获取数据
        // pCurve[i]->addData(cnt, pTxtValueCurve[i]->text().toShort());// 从输入框获取数据
        //  因为20条线重叠在一起，所以QCustomPlot输入为0时看起来像不显示，隐藏其他后观察单条曲线是可以看到显示的
    }

    // 设置x坐标轴显示范围，使其自适应缩放x轴，x轴最大显示pointCountX个点。与chkTrackX复选框有关
    //    if (ui->chkTrackX->checkState())
    //    {
    //        // customPlot->xAxis->setRange((pCurve[0]->dataCount()>pointCountX)?(pCurve[0]->dataCount()-pointCountX):0, pCurve[0]->dataCount());
    //        setAutoTrackX(customPlot);
    //    }
    //    // 设置y坐标轴显示范围，使其自适应曲线缩放
    //    if (ui->chkAdjustY->checkState())
    //    {
    //        setAutoTrackY(customPlot);
    //    }

    // 更新绘图，这种方式在高填充下太浪费资源。有另一种方式rpQueuedReplot，可避免重复绘图。
    // 最好的方法还是将数据填充、和更新绘图分隔开。将更新绘图单独用定时器更新。例程数据量较少没用单独定时器更新，实际工程中建议大家加上。
    // customPlot->replot();
    customPlot->replot(QCustomPlot::rpQueuedReplot);

    static QTime time(QTime::currentTime());
    double key = time.elapsed() / 1000.0; // 开始到现在的时间，单位秒
    ////计算帧数
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key - lastFpsKey > 1) // 每1秒求一次平均值
    {
        // 状态栏显示帧数和数据总数
        ui->statusbar->showMessage(
            QString("%1 FPS, Total Data points: %2")
                .arg(frameCount / (key - lastFpsKey), 0, 'f', 0)
                .arg(customPlot->graph(0)->data()->size() + customPlot->graph(1)->data()->size()),
            0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

// 曲线更新绘图，波形数据绘图（int型）
void Plot::ShowPlot_WaveForm(QCustomPlot *customPlot, int value[])
{
    cnt++;
    for (int i = 0; i < 20; i++)
    {
        // pTxtValueCurve[i]->setText(QString::number(value[i]));
        pCurve[i]->addData(cnt, value[i]);
    }
    //    if (ui->chkTrackX->checkState())
    //    {
    //        setAutoTrackX(customPlot);
    //    }
    //    if (ui->chkAdjustY->checkState())
    //    {
    //        setAutoTrackY(customPlot);
    //    }
    customPlot->replot(QCustomPlot::rpQueuedReplot);
    static QTime time(QTime::currentTime());
    double key = time.elapsed() / 1000.0;
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key - lastFpsKey > 1)
    {
        ui->statusbar->showMessage(
            QString("%1 FPS, Total Data points: %2")
                .arg(frameCount / (key - lastFpsKey), 0, 'f', 0)
                .arg(customPlot->graph(0)->data()->size() + customPlot->graph(1)->data()->size()),
            0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

// 曲线更新绘图，波形数据绘图（float型）
void Plot::ShowPlot_WaveForm(QCustomPlot *customPlot, float value[])
{
    cnt++;
    for (int i = 0; i < 20; i++)
    {
        // pTxtValueCurve[i]->setText(QString::number(value[i]));
        pCurve[i]->addData(cnt, value[i]);
    }
    //    if (ui->chkTrackX->checkState())
    //    {
    //        setAutoTrackX(customPlot);
    //    }
    //    if (ui->chkAdjustY->checkState())
    //    {
    //        setAutoTrackY(customPlot);
    //    }
    customPlot->replot(QCustomPlot::rpQueuedReplot);
    static QTime time(QTime::currentTime());
    double key = time.elapsed() / 1000.0;
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key - lastFpsKey > 1)
    {
        ui->statusbar->showMessage(
            QString("%1 FPS, Total Data points: %2")
                .arg(frameCount / (key - lastFpsKey), 0, 'f', 0)
                .arg(customPlot->graph(0)->data()->size() + customPlot->graph(1)->data()->size()),
            0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

/* 功能：隐藏/显示曲线n
 * QCustomPlot *pPlot：父控件，绘图图表
 * QCPGraph *pCurve：图表的曲线
 * int arg1：曲线的可见性，>0可见，0不可见
 * */
void Plot::curveSetVisible(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1)
{
    if (arg1)
    {
        pCurve->setVisible(true);
    }
    else
    {
        pCurve->setVisible(false);
    }
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

/* 功能：弹出颜色对话框，设置曲线n的颜色
 * QCustomPlot *pPlot：父控件，绘图图表
 * QCPGraph *pCurve：图表的曲线
 * QPushButton *btn：曲线颜色选择框的按键，与曲线的颜色同步
 * */
void Plot::curveSetColor(QCustomPlot *pPlot, QCPGraph *pCurve, QPushButton *btn)
{
    // 获取当前颜色
    QColor bgColor(0, 0, 0);
    // bgColor = btn->palette().color(QPalette::Background);// 由pushButton的背景色获得颜色
    bgColor = pCurve->pen().color(); // 由curve曲线获得颜色
    // 以当前颜色打开调色板，父对象，标题，颜色对话框设置项（显示Alpha透明度通道）
    // QColor color = QColorDialog::getColor(bgColor);
    QColor color = QColorDialog::getColor(bgColor, this,
                                          tr("颜色对话框"),
                                          QColorDialog::ShowAlphaChannel);
    // 判断返回的颜色是否合法。若点击x关闭颜色对话框，会返回QColor(Invalid)无效值，直接使用会导致变为黑色。
    if (color.isValid())
    {
        // 设置选择框颜色
        btn->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(color.name()));
        // 设置曲线颜色
        QPen pen = pCurve->pen();
        pen.setBrush(color);
        pCurve->setPen(pen);
        // pCurve->setPen(QPen(color));
    }
    // 更新绘图
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

/* 功能：加粗显示曲线n
 * QCustomPlot *pPlot：父控件，绘图图表
 * QCPGraph *pCurve：图表的曲线
 * int arg1：曲线的粗细，>0粗，0细
 * */
void Plot::curveSetBold(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1)
{
    // 预先读取曲线的颜色
    QPen pen = pCurve->pen();
    // pen.setBrush(pCurve->pen().color());// 由curve曲线获得颜色

    if (arg1)
    {
        pen.setWidth(3);
        pCurve->setPen(pen);
    }
    else
    {
        pen.setWidth(1);
        pCurve->setPen(pen);
    }
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

/* 功能：选择曲线样式（线，点，积）
 * QCustomPlot *pPlot：父控件，绘图图表
 * QCPGraph *pCurve：图表的曲线
 * int arg1：曲线样式（线，点，积）
 * */
void Plot::curveSetLineStyle(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1)
{
    // 设置曲线样式
    // customPlot->graph(19)->setLineStyle(QCPGraph::lsLine); // 数据点通过直线连接
    // customPlot->graph(19)->setLineStyle((QCPGraph::LineStyle)i);//设置线性
    // pCurve->setLineStyle(QCPGraph::LineStyle(arg1));
    pCurve->setLineStyle((QCPGraph::LineStyle)arg1);
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

/* 功能：选择散点样式（空心圆、实心圆、正三角、倒三角）
 * QCustomPlot *pPlot：父控件，绘图图表
 * QCPGraph *pCurve：图表的曲线
 * int arg1：散点样式（空心圆、实心圆、正三角、倒三角）
 * */
void Plot::curveSetScatterStyle(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1)
{
    // 设置散点样式
    // customPlot->graph(19)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5)); // 空心圆
    // pCurve->setScatterStyle(QCPScatterStyle::ScatterShape(arg1)); // 散点样式
    // pCurve->setScatterStyle((QCPScatterStyle::ScatterShape)arg1); // 散点样式
    if (arg1 <= 10)
    {
        pCurve->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)arg1, 5)); // 散点样式
    }
    else
    {                                                                                     // 后面的散点图形略复杂，太小会看不清
        pCurve->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)arg1, 8)); // 散点样式
    }
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

// 图例显示与否
void Plot::on_chkShowLegend_stateChanged(int arg1)
{
    if (arg1)
    {
        // 显示图表的图例
        pPlot1->legend->setVisible(true);
    }
    else
    {
        // 不显示图表的图例
        pPlot1->legend->setVisible(false);
    }
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

// 设置曲线x轴自动跟随
void Plot::setAutoTrackX(QCustomPlot *pPlot)
{
    //    pointCountX = ui->txtPointCountX->text().toUInt();
    if (pCurve[0]->dataCount() < pointCountX)
    {
        pPlot->xAxis->setRange(0, pointCountX);
    }
    else
    {
        pPlot->xAxis->setRange((pCurve[0]->dataCount() > pointCountX) ? (pCurve[0]->dataCount() - pointCountX) : 0, pCurve[0]->dataCount());
    }
}

// 设置曲线x轴自适应
void Plot::setAutoX(QCustomPlot *pPlot, int xRange)
{
    pointCountX = xRange;
    if (pCurve[0]->dataCount() < pointCountX)
    {
        pPlot->xAxis->setRange(0, pointCountX);
    }
    else
    {
        pPlot->xAxis->setRange((pCurve[0]->dataCount() > pointCountX) ? (pCurve[0]->dataCount() - pointCountX) : 0, pCurve[0]->dataCount());
    }
}

// 设置曲线x轴手动设置范围（依照右下角输入框）
void Plot::setManualSettingX(QCustomPlot *pPlot)
{
    //    pointOriginX = ui->txtPointOriginX->text().toInt();
    //    pointCountX = ui->txtPointCountX->text().toUInt();
    //    pPlot->xAxis->setRange(pointOriginX, pointOriginX + pointCountX);
}

// 设置Y轴自适应
void Plot::setAutoTrackY(QCustomPlot *pPlot)
{
    pPlot->graph(0)->rescaleValueAxis(); // y轴自适应，可放大可缩小
    for (int i = 1; i < 20; i++)
    {
        pPlot->graph(i)->rescaleValueAxis(true); // y轴自适应，只能放大
    }
}

// 重新设置X轴显示的点数
void Plot::on_txtPointCountX_returnPressed()
{
    //    if (ui->chkTrackX->checkState())
    //    {
    //        setAutoTrackX(pPlot1);
    //    }
    //    else
    //    {
    //        setManualSettingX(pPlot1);
    //    }
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

void Plot::on_txtPointCountY_returnPressed()
{
    //    pointCountY = ui->txtPointCountY->text().toUInt();
    //    pPlot1->yAxis->setRange(pointCountY / 2 * -1, pointCountY / 2);
    //    ui->txtPointOriginY->setText(QString::number(pointCountY / 2 * -1));
    //    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

void Plot::on_btnColourBack_clicked()
{
    // 获取当前颜色
    QColor bgColor(0, 0, 0);
    bgColor = ui->btnColourBack->palette().color(QPalette::Background); // 由pushButton的背景色获得颜色
    // 以当前颜色打开调色板，父对象，标题，颜色对话框设置项（显示Alpha透明度通道）
    // QColor color = QColorDialog::getColor(bgColor);
    QColor color = QColorDialog::getColor(bgColor, this,
                                          tr("颜色对话框"),
                                          QColorDialog::ShowAlphaChannel);

    // 判断返回的颜色是否合法。若点击x关闭颜色对话框，会返回QColor(Invalid)无效值，直接使用会导致变为黑色。
    if (color.isValid())
    {
        // 设置背景颜色
        pPlot1->setBackground(color);
        // 设置背景选择框颜色
        ui->btnColourBack->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(color.name()));
    }
    // 更新绘图
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

void Plot::on_txtPointOriginX_returnPressed()
{
    setManualSettingX(pPlot1);
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

void Plot::on_chkTrackX_stateChanged(int arg1)
{
    //    if (arg1)
    //    {
    //        ui->txtPointOriginX->setEnabled(false);
    //        setAutoTrackX(pPlot1);
    //        pPlot1->replot(QCustomPlot::rpQueuedReplot);
    //    }
    //    else
    //    {
    //        ui->txtPointOriginX->setEnabled(true);
    //    }
}

void Plot::on_chkAdjustY_stateChanged(int arg1)
{
    //    if (arg1)
    //    {
    //        ui->txtPointOriginY->setEnabled(false);
    //        ui->txtPointCountY->setEnabled(false);
    //        setAutoTrackY(pPlot1);
    //        pPlot1->replot(QCustomPlot::rpQueuedReplot);
    //    }
    //    else
    //    {
    //        ui->txtPointOriginY->setEnabled(true);
    //        ui->txtPointCountY->setEnabled(true);
    //    }
}

void Plot::on_txtPointOriginY_returnPressed()
{
    //    pointOriginY = ui->txtPointOriginY->text().toInt();
    //    pointCountY = ui->txtPointCountY->text().toUInt();
    //    pPlot1->yAxis->setRange(pointOriginY, pointOriginY + pointCountY);
    //    qDebug() << pointOriginY << pointCountY;
    //    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

void Plot::showDashboard(QCustomPlot *customPlot)
{
    // 生成对象dataParser
    DataParser parsefp;

    if (CurveLineNames.size() > 0)
    {
        // 中文名称设置
        for (int i = 0; i < CurveLineNames.size(); i++)
        {
            CurveLineNamesInChinese << parsefp.parseData(CurveLineNames.at(i));
        }
    }
    else
    {
        PlotError error(PlotError::KnownError, "数据为空！");
        PlotError::debugError(error);
    }

    // 显示图表的图例
    // 只显示前10条图例，如果大于10条的话，
    customPlot->legend->setVisible(true);
    // 设置图例框为透明
    customPlot->legend->setBrush(Qt::NoBrush);
    customPlot->legend->setBorderPen(Qt::NoPen);
    customPlot->legend->setMinimumSize(300, 0);        // 设置最小宽度
    customPlot->legend->setMaximumSize(300, 16777215); // 设置最大宽度
    // 只显示前10条图例，并缩小图例框面积，不显示的曲线不占用空间
    int legendItemCount = 0;
    // for (int i = 0; i < customPlot->graphCount(); ++i)
    // {

    //     bool showLegend = false;
    //     for (int j = 0; j < sizeof(defaultLegendIndex)/sizeof(defaultLegendIndex[0]); ++j)
    //     {
    //         if (i == defaultLegendIndex[j])
    //         {
    //         showLegend = true;
    //         break;
    //         }
    //     }
    //     if (showLegend)
    //     {
    //         customPlot->graph(i)->setVisible(true);
    //         customPlot->legend->item(i)->setVisible(true);
    //         ++legendItemCount;
    //     }
    //     else
    //     {
    //         customPlot->graph(i)->setVisible(true); // 曲线本身可见性不变
    //         customPlot->legend->item(i)->setVisible(false);
    //     }
    // }
    // ...existing code...
    // 只显示defaultLegendIndex中的曲线图例，其它图例项彻底移除
    customPlot->legend->clearItems(); // 先清空所有图例项
    for (int j = 0; j < sizeof(defaultLegendIndex) / sizeof(defaultLegendIndex[0]); ++j)
    {
        int idx = defaultLegendIndex[j];
        if (idx < customPlot->graphCount())
        {
            customPlot->graph(idx)->setVisible(true);
            customPlot->legend->addItem(new QCPPlottableLegendItem(customPlot->legend, customPlot->graph(idx)));
        }
    }
    // ...existing code...
    // TopLegendFlash();
}
QString padString(const QString &str, int totalWidth)
{
    int width = 0;
    for (QChar ch : str)
    {
        width += ch.unicode() < 128 ? 1 : 2; // 英文算1，汉字算2
    }
    int spaces = totalWidth - width;
    if (spaces > 0)
        return str + QString(spaces, ' ');
    else
        return str;
}

void Plot::TopLegendFlash(void)
{

    QString temp;
    QString temp2;
    QStringList templist; // 对齐使用
    DataParser d;
    if (allCurvesInfoText)
    {
        pPlot1->removeItem(allCurvesInfoText);
        allCurvesInfoText = nullptr;
    }
    QString info;
    int maxIndex = pPlot1->graphCount();
    int indexWidth = QString::number(maxIndex).length(); // 序号最大宽度
    for (int i = 0; i < 12; ++i)
    {
        if (i < 8)
        {
            QCPGraph *graph = pPlot1->graph(i);
            QCPGraph *graph2 = pPlot1->graph(i + 12);
            if (!graph || !graph2)
                continue;
            temp = d.removeSpaces(graph->name());
            temp.replace("\n", "");
            temp2 = d.removeSpaces(graph2->name());
            templist << temp + " " + temp2;
        }
        else
        {
            QCPGraph *graph = pPlot1->graph(i);
            if (!graph)
                continue;
            temp = d.removeSpaces(graph->name());
            templist << temp;
        }
    }

    // QFont monoFont("Courier New"); // 或 "Monospace", "Consolas"
    // monoFont.setPointSize(10);

    QFont monoFont("Consolas"); // 或 "Courier New"、"Monaco"
    monoFont.setStyleHint(QFont::Monospace);

    // d.alignString(templist, infoFont); // 按y对齐
    for (int i = 0; i < 12; ++i)
    {
        if (i < 8)
        {
            // info += templist.at(i);
            QRegularExpression re(R"(([^-\d]+)(-?\d+(?:\.\d+)?)\s*([^-\d]+)(-?\d+(?:\.\d+)?))");

            // 执行匹配
            QRegularExpressionMatch match = re.match(templist.at(i));

            if (match.hasMatch())
            {
                QString str1 = match.captured(1); // 第一个字符串
                QString num1 = match.captured(2); // 第一个数字
                QString str2 = match.captured(3); // 第二个字符串
                QString num2 = match.captured(4); // 第二个数字

                qDebug() << "str1:" << str1;
                qDebug() << "num1:" << num1.toDouble();
                qDebug() << "str2:" << str2;
                qDebug() << "num2:" << num2.toDouble();
                // info += QString::asprintf("%-15scc dd%12.2faa  bb%-10scc dd%12.2f\n", str1.toStdString().c_str(), num1.toDouble(), str2.toStdString().c_str(), num2.toDouble());
                // info += QString("%1cc dd%2aa  bb%3cc dd%4\n").arg(str1, -15).arg(num1.toDouble(), 12, 'f', 2).arg(str2, -10).arg(num2.toDouble(), 12, 'f', 2);
                if (i == 0)
                {
                    info += QString("%1  %2 %3 %4\n")
                                .arg(padString(str1, 12))
                                .arg(num1.toInt(), 12)
                                .arg(padString(str2, 12))
                                .arg(num2.toDouble(), 12, 'f', 2);
                }
                else
                {
                    info += QString("%1 %2 %3 %4\n")
                                .arg(padString(str1, 12))
                                .arg(num1.toDouble(), 12, 'f', 2)
                                .arg(padString(str2, 12))
                                .arg(num2.toDouble(), 12, 'f', 2);
                }
            }
        }
        /// else
        // info += templist.at(i);
    }
    allCurvesInfoText = new QCPItemText(pPlot1);
    // 设置无边框
    allCurvesInfoText->setBrush(QBrush(Qt::NoBrush));
    allCurvesInfoText->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
    allCurvesInfoText->position->setType(QCPItemPosition::ptAxisRectRatio);
    allCurvesInfoText->position->setCoords(0.5, 0); // 顶部居中
    allCurvesInfoText->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
    allCurvesInfoText->setText(info);
    allCurvesInfoText->setFont(monoFont);
    allCurvesInfoText->setPen(QPen(Qt::NoPen));

    // 关键：设置文本左对齐
}

// 每次图表重绘后，都会更新当前显示的原点坐标与范围。与上次不同时才会更新显示，解决有曲线数据时无法输入y的参数的问题
void Plot::repPlotCoordinate()
{
    static int xOrigin, yOrigin, yCount;
    static int xOriginLast, yOriginLast, yCountLast;

    xOrigin = pPlot1->xAxis->range().lower;
    yOrigin = pPlot1->yAxis->range().lower;
    yCount = pPlot1->yAxis->range().size();
    // 与上次不同时才会更新显示，解决有曲线数据时无法输入y的参数的问题
    //    if (xOriginLast != xOrigin)
    //    {
    //        ui->txtPointOriginX->setText(QString::number(xOrigin));
    //    }
    //    if (yOriginLast != yOrigin)
    //    {
    //        ui->txtPointOriginY->setText(QString::number(yOrigin));
    //    }
    //    if (yCountLast != yCount)
    //    {
    //        ui->txtPointCountY->setText(QString::number(yCount));
    //    }
    // 记录历史值
    xOriginLast = xOrigin;
    yOriginLast = yOrigin;
    yCountLast = yCount;
}

// 清空绘图
void Plot::on_btnClearGraphs_clicked()
{
    // pPlot1->clearGraphs(); // 清除图表的所有数据和设置，需要重新设置才能重新绘图
    // pPlot1->clearPlottables(); // 清除图表中所有曲线，需要重新添加曲线才能绘图
    for (int i = 0; i < 20; i++)
    {
        pPlot1->graph(i)->data().data()->clear(); // 仅仅清除曲线的数据
    }
    cnt = 0;
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

// 设置X轴主刻度个数
void Plot::on_txtMainScaleNumX_returnPressed()
{
    pPlot1->xAxis->ticker()->setTickCount(ui->txtMainScaleNumX->text().toUInt());
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

// 设置Y轴主刻度个数
void Plot::on_txtMainScaleNumY_returnPressed()
{
    pPlot1->yAxis->ticker()->setTickCount(ui->txtMainScaleNumY->text().toUInt());
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

void Plot::mouseMove2(QMouseEvent *e)
{
    double mouseX = pPlot1->xAxis->pixelToCoord(e->pos().x());
    double vLineX = mouseX;
    QStringList Cvlls = CurveLineNamesInChinese;
    for (int i = 0; i < pPlot1->graphCount(); ++i)
    {
        QCPGraph *graph = pPlot1->graph(i);
        if (!graph)
            continue;
        auto data = graph->data();
        if (data->isEmpty())
            continue;

        // 找到距离mouseX最近的点
        double minDist = 1e100;
        double nearestKey = 0;
        double nearestValue = 0;
        for (auto it = data->constBegin(); it != data->constEnd(); ++it)
        {
            double dist = fabs(it->key - mouseX);
            if (dist < minDist)
            {
                minDist = dist;
                nearestKey = it->key;
                nearestValue = it->value;
            }
        }

        // 更新tracer到最近点
        tracers[i]->setGraphKey(nearestKey);
        tracers[i]->setInterpolating(false); // 关闭插值
        tracers[i]->updatePosition();
        tracers[i]->setVisible(true);

        double yValue = nearestValue;

        if (i < Cvlls.size())
        {
            if (Cvlls[i] == "currentDistance")
                Cvlls[i].append(QString("总距离10米,剩余%1米\n")
                                    .arg(yValue < 0 ? QString::number(yValue, 'f', 2) : " " + QString::number(yValue, 'f', 2)));
            else
                Cvlls[i].append(QString("%1\n")
                                    .arg(yValue < 0 ? QString::number(yValue, 'f', 2) : " " + QString::number(yValue, 'f', 2)));
        }
        vLineX = nearestKey; // 竖线也吸附到最近点
    }
    DataParser d;
    d.alignString(Cvlls, pPlot1->legend->font());
    setCurveslegendName(Cvlls);
    vLine->point1->setCoords(vLineX, pPlot1->yAxis->range().lower);
    vLine->point2->setCoords(vLineX, pPlot1->yAxis->range().upper);
    // TopLegendFlash();
    pPlot1->replot();

    addFrameToWinPlot();
}

void Plot::hideCurve(int index)
{
    if (index < pPlot1->graphCount())
    {
        curveSetVisible(pPlot1, pCurve[index], 0);
        pChkVisibleCurve[index]->setChecked(false);
    }
    else
    {
        QString errorMsg = QString("曲线索引超出范围，最大索引为：%1").arg(pPlot1->graphCount() - 1);
        PlotError error(PlotError::KnownError, errorMsg);
        PlotError::showErrorDialog(this, error);
    }
}
void Plot::showCurve(int index)
{
    if (index < pPlot1->graphCount())
    {
        curveSetVisible(pPlot1, pCurve[index], 1);
        pChkVisibleCurve[index]->setChecked(true);
    }
    else
    {
        QString errorMsg = QString("曲线索引超出范围，最大索引为：%1").arg(pPlot1->graphCount() - 1);
        PlotError error(PlotError::KnownError, errorMsg);
        PlotError::showErrorDialog(this, error);
    }
}

// 参数为 pair<int, int>，分别表示起始点和数量
void Plot::stageDistinguish(std::pair<int, int> range)
{
    int startPoint = range.first + 1;
    int pointCount = range.second;
    qDebug() << "startPoint:" << startPoint << "pointCount:" << pointCount;
    // for(int i = 0; i < pointCount; i++){
    //     qDebug() << "y:" << logDataPtr[startPoint + i].phaseFlag;
    // }
    if (1)
    {
        if (1)
        {
            // 遍历数据，查找y=1,2,3对应的x范围
            QVector<int> x1, x2, x3;
            for (int i = 0; i < pointCount; i++)
            {
                if (logDataPtr[startPoint + i].phaseFlag == 1) // y==1
                    x1.append(i);
                else if (logDataPtr[startPoint + i].phaseFlag == 2) // y==2
                    x2.append(i);
                else if (logDataPtr[startPoint + i].phaseFlag == 3) // y==3
                    x3.append(i);
            }
            DataParser d;
            std::vector<int> x11 = x1.toStdVector();
            std::vector<int> x22 = x2.toStdVector();
            std::vector<int> x33 = x3.toStdVector();
            d.CreatePhaseRange(x11, range1);
            d.CreatePhaseRange(x22, range2);
            d.CreatePhaseRange(x33, range3);

            qDebug() << "range1:" << range1;
            qDebug() << "range2:" << range2;
            qDebug() << "range3:" << range3;

            // 在x范围内画不同颜色的矩形
            if (!x1.isEmpty())
            {
                QCPItemRect *rect1 = new QCPItemRect(pPlot1);
                rect1->topLeft->setType(QCPItemPosition::ptPlotCoords);
                rect1->bottomRight->setType(QCPItemPosition::ptPlotCoords);
                rect1->topLeft->setCoords(x1.first(), 0);
                rect1->bottomRight->setCoords(x1.last(), pPlot1->yAxis->range().lower);
                rect1->setBrush(QBrush(QColor(255, 0, 0, 10))); // 红色
                rect1->setPen(Qt::NoPen);
            }
            if (!x2.isEmpty())
            {
                QCPItemRect *rect2 = new QCPItemRect(pPlot1);
                rect2->topLeft->setType(QCPItemPosition::ptPlotCoords);
                rect2->bottomRight->setType(QCPItemPosition::ptPlotCoords);
                rect2->topLeft->setCoords(x2.first(), 0); // 只到y=0
                rect2->bottomRight->setCoords(x2.last(), pPlot1->yAxis->range().lower);
                rect2->setBrush(QBrush(QColor(0, 255, 0, 10))); // 绿色
                rect2->setPen(Qt::NoPen);
            }
            if (!x3.isEmpty())
            {
                QCPItemRect *rect3 = new QCPItemRect(pPlot1);
                rect3->topLeft->setType(QCPItemPosition::ptPlotCoords);
                rect3->bottomRight->setType(QCPItemPosition::ptPlotCoords);
                rect3->topLeft->setCoords(x3.first(), 0); // 只到y=0
                rect3->bottomRight->setCoords(x3.last(), pPlot1->yAxis->range().lower);
                rect3->setBrush(QBrush(QColor(0, 0, 255, 10))); // 蓝色
                rect3->setPen(Qt::NoPen);
            } 
            
            
            
            
            
            
            QVector<int> x4, x5, x6, x7;
            for (int i = 0; i < pointCount; i++)
            {
                qDebug() << "y:" << logDataPtr[startPoint + i].firstPhaseCount;
                if (logDataPtr[startPoint + i].firstPhaseCount == 1) // y==1
                    x4.append(i);
                else if (logDataPtr[startPoint + i].firstPhaseCount == 2) // y==2
                    x5.append(i);
                else if (logDataPtr[startPoint + i].firstPhaseCount == 3) // y==3
                    x6.append(i);
                else if (logDataPtr[startPoint + i].firstPhaseCount == 4) // y==4
                    x7.append(i);
            }

            // 在x范围内画不同颜色的矩形
            if (!x4.isEmpty())
            {
                QCPItemRect *rect4 = new QCPItemRect(pPlot1);
                rect4->topLeft->setType(QCPItemPosition::ptPlotCoords);
                rect4->bottomRight->setType(QCPItemPosition::ptPlotCoords);
                rect4->topLeft->setCoords(x4.first(), pPlot1->yAxis->range().upper);
                rect4->bottomRight->setCoords(x4.last(), 0);
                rect4->setBrush(QBrush(QColor(255, 0, 0, 10))); // 红色
                rect4->setPen(Qt::NoPen);
            }
            if (!x5.isEmpty())
            {
                QCPItemRect *rect5 = new QCPItemRect(pPlot1);
                rect5->topLeft->setType(QCPItemPosition::ptPlotCoords);
                rect5->bottomRight->setType(QCPItemPosition::ptPlotCoords);
                rect5->topLeft->setCoords(x5.first(), pPlot1->yAxis->range().upper); // 只到y=0
                rect5->bottomRight->setCoords(x5.last(), 0);
                rect5->setBrush(QBrush(QColor(0, 255, 0, 10))); // 绿色
                rect5->setPen(Qt::NoPen);
            }
            if (!x6.isEmpty())
            {
                QCPItemRect *rect6 = new QCPItemRect(pPlot1);
                rect6->topLeft->setType(QCPItemPosition::ptPlotCoords);
                rect6->bottomRight->setType(QCPItemPosition::ptPlotCoords);
                rect6->topLeft->setCoords(x6.first(), pPlot1->yAxis->range().upper); // 只到y=0
                rect6->bottomRight->setCoords(x6.last(), 0);
                rect6->setBrush(QBrush(QColor(0, 0, 255, 10))); // 蓝色
                rect6->setPen(Qt::NoPen);
            }
            if (!x7.isEmpty())
            {
                QCPItemRect *rect7 = new QCPItemRect(pPlot1);
                rect7->topLeft->setType(QCPItemPosition::ptPlotCoords);
                rect7->bottomRight->setType(QCPItemPosition::ptPlotCoords);
                rect7->topLeft->setCoords(x7.first(), pPlot1->yAxis->range().upper); // 只到y=0
                rect7->bottomRight->setCoords(x7.last(), 0);
                rect7->setBrush(QBrush(QColor(255, 255, 0, 10))); // 黄色
                rect7->setPen(Qt::NoPen);
            }


        }
        pPlot1->replot(QCustomPlot::rpQueuedReplot);
    }

}

void Plot::on_pushButton_released()
{
    // 获取按钮的名称
    QString buttonText = ui->pushButton->text();
    if (buttonText == "<")
    {
        ui->pushButton->setText(">");
        // 展开区域
        ui->frame_2->setVisible(true);
    }
    else
    {
        ui->pushButton->setText("<");
        // 收起区域
        ui->frame_2->setVisible(false);
    }
}

void Plot::on_tabWidget_currentChanged(int index)
{
    if (index == 1)
    {
        // 随便导入表信息测试一下
        // if (ui->gps_table) {

        //     // 创建一个标准模型
        //     QStandardItemModel *model = new QStandardItemModel(2, 3, this);
        //     model->setHorizontalHeaderLabels(QStringList() << "经度" << "纬度" << "时间");
        //     model->setItem(0, 0, new QStandardItem("120.123456"));
        //     model->setItem(0, 1, new QStandardItem("30.654321"));
        //     model->setItem(0, 2, new QStandardItem("2024-06-01 12:00:00"));
        //     model->setItem(1, 0, new QStandardItem("120.654321"));
        //     model->setItem(1, 1, new QStandardItem("30.123456"));
        //     model->setItem(1, 2, new QStandardItem("2024-06-01 12:01:00"));
        //     ui->gps_table->setModel(model);
        // }
        testLogDataPtr();
        if (selectedIndex != -1)
        {
            showGroupToTable();
        }
    }
}

// 测试函数：读取logDataPtr并输出部分内容
void Plot::testLogDataPtr()
{
    int i = 0;
    while (plot_group_index[i].first != 0 || plot_group_index[i].second != 0)
    {
        qDebug() << "plot_group_index[" << i << "]:" << plot_group_index[i].first << " " << plot_group_index[i].second;
        i++;
    }
    qDebug() << "一共" << i << "组";
}
void Plot::setSelectedGroup(int index)
{
    selectedIndex = index;
}

void Plot::showGroupToTable()
{
    if (!logDataPtr || !plot_group_index || selectedIndex < 0)
        return;
    int start = plot_group_index[selectedIndex].first + 1; //
    int count = plot_group_index[selectedIndex].second;
    if (count <= 0)
        return;

    QStandardItemModel *model = new QStandardItemModel(count, 3, this);
    model->setHorizontalHeaderLabels(QStringList() << "时间戳" << "经度" << "纬度");

    for (int i = 0; i < count; ++i)
    {

        int idx = start + i;

        qDebug() << "====================";
        qDebug() << "timestamp" << logDataPtr[idx].timestamp << "longitude" << logDataPtr[idx].longitude << "latitude" << logDataPtr[idx].latitude;

        model->setItem(i, 0, new QStandardItem(QString::number(logDataPtr[idx].timestamp)));
        model->setItem(i, 1, new QStandardItem(QString::number(logDataPtr[idx].longitude, 'f', DECIMAL_COUNT_FOR_JW))); // 保留6位小数
        model->setItem(i, 2, new QStandardItem(QString::number(logDataPtr[idx].latitude, 'f', DECIMAL_COUNT_FOR_JW)));
    }
    if (ui->gps_table)
    {
        ui->gps_table->setModel(model);
        ui->gps_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

void Plot::on_plottest_button_released()
{
}

void Plot::setPid(int index, float kp, float ki, float kd, float integralLimit)
{
    QString str;
    str = QString("kp:%1 ki:%2 kd:%3 iLim:%4").arg(kp,4).arg(ki,4).arg(kd,4).arg(integralLimit,4);
    switch (index)
    {
    case 0:
        ui->first_pid->setText("PID1 " + str);
        break;
    case 1:
        ui->second_pid->setText("PID2 " + str);
        break;
    case 2:
        ui->third_pid->setText("PID3 " + str);
        break;
    default:
        break;
    }
}

void Plot::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control && !event->isAutoRepeat())
    {
        // Ctrl键按下，设置状态
        pPlot1->axisRect()->setRangeZoom(Qt::Horizontal); // 水平方向缩放
    }
    QMainWindow::keyPressEvent(event); // 传递事件
}

void Plot::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control && !event->isAutoRepeat())
    {
        // Ctrl键松开，恢复状态
        pPlot1->axisRect()->setRangeZoom(Qt::Vertical); // 竖直方向缩放
    }
    QMainWindow::keyReleaseEvent(event); // 传递事件
}

void Plot::onPlotClicked(QMouseEvent *event)
{
    // 示例：获取点击位置的像素坐标和轴坐标
    if (!pPlot1)
        return;
    QString timestamp;
    // 检查 Alt 键是否为按下状态
    if (QApplication::keyboardModifiers() & Qt::AltModifier)
    {
        Skip_Enable = true;
    }
    else
    {
        Skip_Enable = false;
    }

    if (Skip_Enable)
    {
        qDebug() << "Skip_Enable";
        if (labels.size() > 2)
        {
            timestamp = labels[1]->text().remove(' ');
            qDebug() << "timestamp:" << timestamp ;
        }
        else
        {
            PlotError error(PlotError::KnownError, "allCurvesInfoText为空");
            PlotError::showErrorDialog(this, error);
        }
        if (g_mainWindow->windowState() & Qt::WindowMinimized)
        {
            g_mainWindow->setWindowState(Qt::WindowNoState);
        }
        g_mainWindow->raise();
        g_mainWindow->scrollToString(timestamp);
    }
}
