#include "filehelper.h"
#include <QDateTime>
#include <QTextStream>

bool FileHelper::exists(const QString &filePath)
{
    QFileInfo info(filePath);
    return info.exists() && info.isFile();
}

QByteArray FileHelper::readAll(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();
    return file.readAll();
}

bool FileHelper::writeAll(const QString &filePath, const QByteArray &data)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    return file.write(data) == data.size();
}

bool FileHelper::append(const QString &filePath, const QByteArray &data)
{
    QFile file(filePath);
    if (!file.open(QIODevice::Append))
        return false;
    return file.write(data) == data.size();
}

bool FileHelper::remove(const QString &filePath)
{
    QFile file(filePath);
    return file.remove();
}

QStringList FileHelper::findAllTxtFiles(const QString &dirPath)
{
    QDir directory(QCoreApplication::applicationDirPath() + "/" + dirPath);
    return directory.entryList(QStringList() << "*.txt", QDir::Files);
}

bool FileHelper::removeTxtFile(const QString &fileName, const QString &dirPath)
{
    QDir dir(QCoreApplication::applicationDirPath() + "/" + dirPath);
    return dir.remove(fileName);
}

QString FileHelper::saveTxtFile(const QString &content, const QString &dirPath)
{
    QString fileName = QCoreApplication::applicationDirPath() + "/" + dirPath + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << content;
        file.close();
        return fileName;
    }
    return QString();
}
int FileHelper::readtxtFile(const QString &fileName, const QString &dirPath, QString &readtxt)
{

    QString filePath = QCoreApplication::applicationDirPath() + "/" + dirPath + "/" + fileName;

    // 读取文件内容并显示在 textEdit 控件上
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        readtxt = in.readAll();
        file.close();
        return FILE_OK;
    }
    else
    {
        return FILE_ERR;
    }
}
