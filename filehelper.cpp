#include "filehelper.h"
#include <QFile>
#include <QFileInfo>

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
