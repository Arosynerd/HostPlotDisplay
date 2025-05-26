/*
这是一个用于解析数据的类。



*/

#include "new_data_parser.h"

// 实现数据解析类的成员函数
DataParser::DataParser()
{
    // 构造函数初始化
}

// 解析数据的具体实现
QByteArray DataParser::parseData(const QByteArray &rawData)
{
    // 这里添加具体的数据解析逻辑
    return rawData; // 示例返回原始数据，需根据实际需求修改
}
/*
    功能：传入一个字符串，返回一个字符串
    输入：rawData
    输出：解析后的数据
    说明：输入英文，翻译成中文
*/
QString DataParser::parseData(const QString &rawData)
{
    if (rawData == "id")
    {
        return "编号";
    }
    else if (rawData == "timestamp")
    {
        return "时间戳";
    }
    else if (rawData == "currentMode")
    {
        return "返航模式";
    }
    else if (rawData == "phaseFlag")
    {
        return "第几阶段";// 第一阶段，第二阶段，第三阶段
    }
    else if (rawData == "goDestSpeed")
    {
        return "基础油门";
    }
    else if (rawData == "firstPhaseCount")
    {
        return "第一阶段次数";
    }
    else if (rawData == "originBearing")
    {
        return "起始方位";
    }
    else if (rawData == "currentBearing")
    {
        return "当前方位";
    }
    else if (rawData == "currentYaw")
    {
        return "当前航向角";
    }
    else if (rawData == "currentDistance")
    {
        return "当前距离";
    }
    else if (rawData == "lineSeparation")
    {
        return "航线间距";
    }
    else if (rawData == "bearingError")
    {
        return "航向误差";
    }
    else if (rawData == "yawCurrentBearing")
    {
        return "当前航向与方位偏差";
    }
    else if (rawData == "rudderAngle")
    {
        return "控制量";
    }
    else if (rawData == "motorSpeedLeft")
    {
        return "左电机油门";
    }
    else if (rawData == "motorSpeedRight")
    {
        return "右电机油门";
    }
    else if (rawData == "kp_yaw_first")
    {
        return "第一阶段Kp";
    }
    else if (rawData == "ki_yaw_first")
    {
        return "第一阶段Ki";
    }
    else if (rawData == "kd_yaw_first")
    {
        return "第一阶段Kd";
    }
    else if (rawData == "integralLimit_yaw_first")
    {
        return "第一阶段积分限幅";
    }
    else if (rawData == "kp_pos")
    {
        return "第二阶段Kp";
    }
    else if (rawData == "ki_pos")
    {
        return "第二阶段Ki";
    }
    else if (rawData == "kd_pos")
    {
        return "第三阶段Kd";
    }
    else if (rawData == "integralLimit_pos")
    {
        return "第二阶段积分限幅";
    }
    else if (rawData == "kp_yaw_third")
    {
        return "第三阶段Kp";
    }
    else if (rawData == "ki_yaw_third")
    {
        return "第三阶段Ki";
    }
    else if (rawData == "kd_yaw_third")
    {
        return "第三阶段Kd";
    }
    else if (rawData == "integralLimit_yaw_third")
    {
        return "第三阶段积分限幅";
    }
    else if (rawData == "latitude")
    {
        return "纬度";
    }
    else if (rawData == "longitude")
    {
        return "经度";
    }
    else if (rawData == "speed")
    {
        return "速度";
    }
    else if (rawData == "kp_angle")
    {
        return "航向角Kp";
    }
    else if (rawData == "minYawDeviation")
    {
        return "最小航向角差";
    }
    else if (rawData == "maxYawDeviation")
    {
        return "最大航向角差";
    }
    else if (rawData == "yawDeviation")
    {
        return "航向角差";
    }
    else if (rawData == "imuYaw")
    {
        return "IMU航向角";
    }
    else if (rawData == "ddmYaw")
    {
        return "DDM航向角";
    }
    else if (rawData == "gpsYaw")
    {
        return "GPS航向角";
    }
    else
    {
        return "未知字段";
    }
}

void DataParser::parseData(const QString &plainText, std::pair<int, int> group_index[], GODEST_log_data_t logData[], int &GroupCount, int &group_count, int &idx_index)
{
    // 输出plainText的前100个字符内容
    QString preview = plainText.left(100);
    qDebug() << "plainText前100个字符:" << preview;

    QStringList lines = plainText.split('\n');

    qDebug() << "查找每两个\"EVENT: 201\"之间的内容";
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
                    //qDebug() << "phaseFlag" << logData[idx_index + innerGroupCount].phaseFlag << "currentDistance:" << logData[idx_index + innerGroupCount].currentDistance << "lineSeparation:" << logData[idx_index + innerGroupCount].lineSeparation << "rudderAngle:" << logData[idx_index + innerGroupCount].rudderAngle << "motorSpeedLeft:" << logData[idx_index + innerGroupCount].motorSpeedLeft << "motorSpeedRight:" << logData[idx_index + innerGroupCount].motorSpeedRight;
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
                    qDebug() << "phaseFlag" << logData[idx_index + innerGroupCount].phaseFlag << "currentDistance:" << logData[idx_index + innerGroupCount].currentDistance << "lineSeparation:" << logData[idx_index + innerGroupCount].lineSeparation << "rudderAngle:" << logData[idx_index + innerGroupCount].rudderAngle << "motorSpeedLeft:" << logData[idx_index + innerGroupCount].motorSpeedLeft << "motorSpeedRight:" << logData[idx_index + innerGroupCount].motorSpeedRight;

                    GroupCount++;
                }
            }
            group_index[group_count].first = idx_index;
            group_index[group_count].second = innerGroupCount;
            qDebug() << QString("最后一组\"EVENT: 201\"到结尾的内容的数量: %1").arg(GroupCount);
        }
    }
}
void DataParser::test(void)
{
    qDebug() << "DataParser test";
}
DataParser::~DataParser()
{
    // 析构函数实现，如有资源释放可在此添加
}
