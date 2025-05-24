/*
这是一个用于解析数据的类。



*/

#include "new_data_parser.h"

// 实现数据解析类的成员函数
DataParser::DataParser() {
    // 构造函数初始化
}

// 解析数据的具体实现
QByteArray DataParser::parseData(const QByteArray& rawData) {
    // 这里添加具体的数据解析逻辑
    return rawData; // 示例返回原始数据，需根据实际需求修改
}
/*
    功能：传入一个字符串，返回一个字符串
    输入：rawData
    输出：解析后的数据
    说明：输入英文，翻译成中文
*/
QString DataParser::parseData(const QString& rawData){
    if (rawData == "id") {
        return "编号";
    } else if (rawData == "timestamp") {
        return "时间戳";
    } else if (rawData == "currentMode") {
        return "当前模式";
    } else if (rawData == "phaseFlag") {
        return "阶段标志";
    } else if (rawData == "goDestSpeed") {
        return "目标速度";
    } else if (rawData == "firstPhaseCount") {
        return "第一阶段计数";
    } else if (rawData == "originBearing") {
        return "起始航向";
    } else if (rawData == "currentBearing") {
        return "当前航向";
    } else if (rawData == "currentYaw") {
        return "当前偏航角";
    } else if (rawData == "currentDistance") {
        return "当前距离";
    } else if (rawData == "lineSeparation") {
        return "航线间距";
    } else if (rawData == "bearingError") {
        return "航向误差";
    } else if (rawData == "yawCurrentBearing") {
        return "偏航-当前航向";
    } else if (rawData == "rudderAngle") {
        return "舵角";
    } else if (rawData == "motorSpeedLeft") {
        return "左电机速度";
    } else if (rawData == "motorSpeedRight") {
        return "右电机速度";
    } else if (rawData == "kp_yaw_first") {
        return "第一阶段偏航KP";
    } else if (rawData == "ki_yaw_first") {
        return "第一阶段偏航KI";
    } else if (rawData == "kd_yaw_first") {
        return "第一阶段偏航KD";
    } else if (rawData == "integralLimit_yaw_first") {
        return "第一阶段偏航积分限幅";
    } else if (rawData == "kp_pos") {
        return "位置KP";
    } else if (rawData == "ki_pos") {
        return "位置KI";
    } else if (rawData == "kd_pos") {
        return "位置KD";
    } else if (rawData == "integralLimit_pos") {
        return "位置积分限幅";
    } else if (rawData == "kp_yaw_third") {
        return "第三阶段偏航KP";
    } else if (rawData == "ki_yaw_third") {
        return "第三阶段偏航KI";
    } else if (rawData == "kd_yaw_third") {
        return "第三阶段偏航KD";
    } else if (rawData == "integralLimit_yaw_third") {
        return "第三阶段偏航积分限幅";
    } else if (rawData == "latitude") {
        return "纬度";
    } else if (rawData == "longitude") {
        return "经度";
    } else if (rawData == "speed") {
        return "速度";
    } else if (rawData == "kp_angle") {
        return "角度KP";
    } else if (rawData == "minYawDeviation") {
        return "最小偏航偏差";
    } else if (rawData == "maxYawDeviation") {
        return "最大偏航偏差";
    } else if (rawData == "yawDeviation") {
        return "偏航偏差";
    } else if (rawData == "imuYaw") {
        return "IMU偏航角";
    } else if (rawData == "ddmYaw") {
        return "DDM偏航角";
    } else if (rawData == "gpsYaw") {
        return "GPS偏航角";
    } else {
        return "未知字段";
    }
}
void DataParser::test(void)
{
    qDebug() << "DataParser test";
}
DataParser::~DataParser() {
    // 析构函数实现，如有资源释放可在此添加
}
