#ifndef PLOT_H
#define PLOT_H

#include <QMainWindow>
#include "qcustomplot.h"
#include <QVector>
#include <QKeyEvent>
//数据解析类
#include "new_data_parser.h"
//报错类
#include "ploterror.h"
extern QStringList CurveLineNames;


// 小数点保留位名称
#define DECIMAL_COUNT_FOR_JW 6




namespace Ui {
class Plot;
}

class Plot : public QMainWindow
{
    Q_OBJECT

public:
    explicit Plot(GODEST_log_data_t* logDataPtr, std::pair<int, int> group_index[100],QWidget *parent = nullptr);
    ~Plot();
    // 绘图控件的指针
    QCustomPlot *pPlot1;

    // 新增鼠标原点那些
    QCPItemTracer *tracer;
    QVector<QCPItemTracer*> tracers;
    QCPItemText *tracerLabel;
    QCPItemStraightLine *vLine = nullptr; // 竖线

    GODEST_log_data_t* logDataPtr;
    std::pair<int, int>* plot_group_index; //修改为指针类型
    /*
    闭区间，
    三个阶段的x轴范围
    */
    std::pair<int,int> range1;
    std::pair<int,int> range2;
    std::pair<int,int> range3;
    
    int selectedIndex = -1;

    QCPItemText *allCurvesInfoText = nullptr;

    void ShowPlot_TimeDemo(QCustomPlot *customPlot, double num);
    void ShowPlot_WaveForm(QCustomPlot *customPlot, short value[]);
    void ShowPlot_WaveForm(QCustomPlot *customPlot, int value[]);
    void ShowPlot_WaveForm(QCustomPlot *customPlot, float value[]);
    void setAutoX(QCustomPlot *pPlot,int xRange);
    void setCurvesName(QStringList lineNames);
    void addCurvesName(QStringList lineNames);
    void hideCurve(int index);
    void stageDistinguish(void);

    void testLogDataPtr();
    void setSelectedGroup(int index);
    void showGroupToTable();
    void setPid(int index, float kp, float ki, float kd, float integralLimit);
    void setCurveslegendName(QStringList lineNames);
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
public slots:
    void mouseMove2(QMouseEvent *e);
private slots:
    void TimeData_Update(void);

    void on_chkShowLegend_stateChanged(int arg1);

    void on_chkDrawDemo_stateChanged(int arg1);

    void on_txtPointCountX_returnPressed();

    void on_txtPointCountY_returnPressed();

    void on_btnColourBack_clicked();

    void on_txtPointOriginX_returnPressed();

    void on_chkTrackX_stateChanged(int arg1);

    void on_chkAdjustY_stateChanged(int arg1);

    void on_txtPointOriginY_returnPressed();

    void repPlotCoordinate();

    void on_btnClearGraphs_clicked();

    void on_txtMainScaleNumX_returnPressed();

    void on_txtMainScaleNumY_returnPressed();

    void on_tabWidget_currentChanged(int index);

    void on_pushButton_released();

    void on_plottest_button_released();

private:
    Ui::Plot *ui;

    // 状态栏指针
    QStatusBar *sBar;
    // 定时器指针
    QTimer *timer;
    // 绘图控件中曲线的指针
    QCPGraph *pCurve[20];
    // 数据解析对象指针
    DataParser *parsefp = NULL;// 数据解析类
    // 绘图框X轴显示的坐标点数

    int pointOriginX=0;
    int pointOriginY=0;
    int pointCountX=0;
    int pointCountY=0;

    double cnt=0;// 当前绘图的X坐标

    // ui界面中，选择曲线可见性的checkBox的指针。方便用指针数组写代码，不然很占地方
    QCheckBox *pChkVisibleCurve[20];
    // ui界面中，选择曲线颜色的pushButton的指针。方便用指针数组写代码，不然很占地方
    QPushButton *pBtnColourCurve[20];
    // ui界面中，曲线当前值的lineEdit的指针。方便用指针数组写代码，不然很占地方
    QLineEdit *pTxtValueCurve[20];
    // ui界面中，选择曲线粗细的radioButton的指针。方便用指针数组写代码，不然很占地方
    QRadioButton *pRdoBoldCurve[20];
    // ui界面中，选择曲线样式的cmbLineStyle的指针。方便用指针数组写代码，不然很占地方
    QComboBox *pCmbLineStyle[20];
    // ui界面中，选择散点样式的cmbScatterStyle的指针。方便用指针数组写代码，不然很占地方
    QComboBox *pCmbScatterStyle[20];

    void QPlot_init(QCustomPlot *customPlot);
    void QPlot_widget_init(void);

    void curveSetVisible(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1);
    void curveSetColor(QCustomPlot *pPlot, QCPGraph *pCurve, QPushButton *btn);
    void curveSetBold(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1);
    void curveSetLineStyle(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1);
    void curveSetScatterStyle(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1);

    void setAutoTrackX(QCustomPlot *pPlot);

    void setManualSettingX(QCustomPlot *pPlot);
    void setAutoTrackY(QCustomPlot *pPlot);

    void showDashboard(QCustomPlot *customPlot);

    void TopLegendFlash(void);
};

#endif // PLOT_H
