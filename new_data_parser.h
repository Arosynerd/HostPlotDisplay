/*
这是一个用于解析数据的类。



*/

#ifndef NEW_DATA_PARSER_H
#define NEW_DATA_PARSER_H

#include <QByteArray>
#include <QDebug>

typedef struct GODEST_log_data_t {
    int id;
    int timestamp;
    int currentMode;
    int phaseFlag;
    int goDestSpeed;
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
    int motorSpeedRight;
    float kp_yaw_first;
    float ki_yaw_first;
    float kd_yaw_first;
    int integralLimit_yaw_first;
    float kp_pos;
    float ki_pos;
    float kd_pos;
    int integralLimit_pos;
    float kp_yaw_third;
    float ki_yaw_third;
    float kd_yaw_third;
    int integralLimit_yaw_third;
    double latitude;
    double longitude;
    double speed;
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

};

#endif // NEW_DATA_PARSER_H
