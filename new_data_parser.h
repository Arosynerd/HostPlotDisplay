/*
这是一个用于解析数据的类。



*/

#ifndef NEW_DATA_PARSER_H
#define NEW_DATA_PARSER_H

#include <QByteArray>
#include <QDebug>
#include <QFontMetrics>
#include "ploterror.h"
#include <QStandardItemModel>
#include <QRegularExpression> // 正则

//#define DEBUG 1
#define ZOOMSTANDARD 64
#define RAW "raw" // 原始数据的目录
typedef struct GODEST_log_data_t {
    int id;
    int timestamp;
    int currentMode;//
    int phaseFlag;
    int goDestSpeed;// 基础油门
    int firstPhaseCount;
    float originBearing;
    float currentBearing;
    float currentYaw;
    float currentDistance;
    float lineSeparation;
    float bearingError;
    float yawCurrentBearing;
    float rudderAngle;
    int motorSpeedLeft;
    int motorSpeedRight;//16
    float kp_yaw_first;//17
    float ki_yaw_first;//18
    float kd_yaw_first;//19
    int integralLimit_yaw_first;//20
    float kp_pos;
    float ki_pos;
    float kd_pos;//23
    int integralLimit_pos;
    float kp_yaw_third;
    float ki_yaw_third;
    float kd_yaw_third;
    int integralLimit_yaw_third;//28
    double latitude;//29
    double longitude;//30
    double speed;//31
    float kp_angle;
    float minYawDeviation;
    float maxYawDeviation;
    float yawDeviation;
    float imuYaw;
    float ddmYaw;
    float gpsYaw;
} GODEST_log_data_t;





class DataParser {
public:
    DataParser();
    ~DataParser();
    QByteArray parseData(const QByteArray& rawData);
    QString parseData(const QString& rawData);
    void test(void);
    void parseData(const QString &plainText, std::pair<int, int> group_index[],GODEST_log_data_t logData[],int &GroupCount, int &group_count, int &idx_index);
    void CreatePhaseRange(const std::vector<int>& vec, std::pair<int, int>& result);
    void alignString(QStringList& okstr,QFont font);
    QString removeSpaces(const QString &input);
    static QString getLastline(const QString &input);
    static float getNumber(const QString &input,int index);
};

#endif // NEW_DATA_PARSER_H
