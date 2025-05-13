#ifndef NEW_DATA_PARSER_H
#define NEW_DATA_PARSER_H

#include <QByteArray>
#include <QDebug>

class DataParser {
public:
    DataParser();
    QByteArray parseData(const QByteArray& rawData);
    void test(void);
};

#endif // NEW_DATA_PARSER_H
