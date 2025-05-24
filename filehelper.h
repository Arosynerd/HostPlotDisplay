#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>

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
    // 查找当前目录下所有txt文件并返回文件名列表
    static QStringList findAllTxtFiles();
    // 删除应用程序目录下指定txt文件
    static bool removeTxtFile(const QString &fileName);
    // 保存内容到应用程序目录下新建txt文件，返回文件名
    static QString saveTxtFile(const QString &content);
};

#endif // FILEHELPER_H
