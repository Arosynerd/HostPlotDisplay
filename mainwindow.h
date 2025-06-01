#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QPainter>
#include "plot.h"

//其他头
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableView>
#include <QHeaderView>
#include <QDir>
#include <QDebug>
//数据解析
#include "new_data_parser.h"
//错误提示
#include "ploterror.h"
//文件方法
#include "filehelper.h"
// 接收缓冲区大小，单位字节
#define BufferSize      50
// 最大帧长度，单位字节
#define MaxFrameLength	40+5			// 对最大帧长度加以限定，防止接收到过长的帧数据
// 完整的帧头，2个字节
#define Frame_Header1   0x3A                    // 串口接收消息包的帧头的第1个字节
#define Frame_Header2   0x3B                    // 串口接收消息包的帧头的第2个字节
// 完整的帧尾，2个字节
#define Frame_Tail1     0x7E                    // 串口接收消息包的帧尾的第1个字节
#define Frame_Tail2     0x7F                    // 串口接收消息包的帧尾的第2个字节

// 功能字1，0x01，自定义波形显示
#define FunWord_WF      0x01
// 功能字2，0x02，信息绘图页面显示，暂时未用
#define FunWord_SM      0x02
// 帧数据中包含有效字节的最大长度
#define ValidByteLength	40			// 对最大帧长度加以限定，防止接收到过长的帧数据

#define NO_FILE_SELECTED 9527
#define FILE1_SELECTED 0
#define FILE2_SELECTED 1
#define FILE3_SELECTED 2
#define FILE4_SELECTED 3
#define FILE5_SELECTED 4
#define FILE6_SELECTED 5






QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 绘图事件
    void paintEvent(QPaintEvent *) override;

    void Open_Serial(QString spTxt);

    void FilesReflash(void);

    void scrollToString(const QString &targetString);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    // 重写鼠标按下事件
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void on_btnSwitch_clicked();

    void serialPortRead_SlotForPlot(QByteArray recBuf);//新增，用于画曲线

    void serialPortRead_Slot();

    void on_btnClearRec_clicked();

    void on_chkRec_stateChanged(int arg1);

    void on_pushButton_clicked();

    void dataRateCalculate(void);

    void dataShow(void); // 新增 绘制曲线

    void on_pushButton_3_released();

    void on_pushButton_4_released();

    void on_tableView_clicked(const QModelIndex &index); // 新增的槽函数

    void onKey0Pressed(); // 处理数字键0按下事件

    void onKey1Pressed(); // 处理数字键1按下事件

    void onKey2Pressed();

    void onKey3Pressed();

    void onKey4Pressed();

    void on_inandoutButton_released();

    void on_TestButton_released();



    void onKey5Pressed();
    void on_checkBox_stateChanged(int arg1);


    void onKey6Pressed();

    void onKey7Pressed();

    void onKey8Pressed();

    void on_pushButton_2_released();

private:
    Ui::MainWindow *ui;
    // 波形绘图窗口
    Plot *plot = NULL;// 必须初始化为空，否则后面NEW判断的时候会异常结束

    QSerialPort *mySerialPort;

    DataParser *dataParser = NULL;// 数据解析类

    // 发送、接收字节计数
    long sendNum=0, recvNum=0, tSend=0, tRecv=0;// 发送/接收数量，历史发送/接收数量，Byte
    long sendRate=0, recvRate=0;// 发送/接收速率，Byte/s
    long recvFrameRate=0, recvErrorNum=0, tFrame=0;// 接收的有效帧数，帧速率，误码帧数量，历史帧数量
    QLabel *lblSendNum, *lblRecvNum, *lblRecvRate,*PortSelected;
    QString strPortSelected = "NoPort";
    void setNumOnLabel(QLabel *lbl, QString strS, long num);
    void setStrOnLabel(QLabel *lbl, QString strS, QString str);
    void LabelUpdate(void);
    /* 与帧过滤有关的标志位 */
    //int snum = 0;                               // 系统串口接收缓存区的可用字节数
    int tnum = 0;                               // 用户串口接收缓存的指针位置
    unsigned char chrtmp[BufferSize];           // 用户串口接收缓存，将缓存的数据放入这里处理
    int f_h1_flag = 0;                          // 接收到帧头的第一个字节标志位
    int f_h_flag = 0;                           // 接收到帧头标志位
    int f_t1_flag = 0;                          // 接收到帧尾的第一个字节标志位
    // 即是标志位，也包含信息
    int f_fun_word = 0;                         // 功能字，限定为0x01、0x02
    int f_length = 0;                           // 帧数据中包含有效字节的长度
    int data_send_count = 0;                    // 发送数据的计数,用于绘制波形
    int file_selected = NO_FILE_SELECTED;                     // 选择的文件 0为未选中,用于删除文件
    //QByteArray xFrameDataFilter(QByteArray *str);

    // 定时发送-定时器
    QTimer *timSend;
    // 发送速率、接收速率统计-定时器
    QTimer *timRate;
    
    QTimer *timTest;

    QByteArray receivedData;

    QByteArray sendDataFrame;

    GODEST_log_data_t logData[8192];//1024 * 8
    
    std::pair<int, int> group_index[100];
};

extern MainWindow *g_mainWindow; // 声明全局主窗口指针
#endif // MAINWINDOW_H
