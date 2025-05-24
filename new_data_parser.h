/*
这是一个用于解析数据的类。



*/

#ifndef NEW_DATA_PARSER_H
#define NEW_DATA_PARSER_H

#include <QByteArray>
#include <QDebug>

class DataParser {
public:
    DataParser();
    ~DataParser();
    QByteArray parseData(const QByteArray& rawData);
    QString parseData(const QString& rawData);
    void test(void);
};

#endif // NEW_DATA_PARSER_H
