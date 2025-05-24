#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QString>
#include <QByteArray>

class FileHelper
{
public:
    // 判断文件是否存在
    static bool exists(const QString &filePath);
    // 读取文件内容
    static QByteArray readAll(const QString &filePath);
    // 写入文件内容（覆盖）
    static bool writeAll(const QString &filePath, const QByteArray &data);
    // 追加内容到文件
    static bool append(const QString &filePath, const QByteArray &data);
    // 删除文件
    static bool remove(const QString &filePath);
};

#endif // FILEHELPER_H
