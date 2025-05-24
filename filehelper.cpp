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

QStringList FileHelper::findAllTxtFiles()
{
    QDir directory(QCoreApplication::applicationDirPath());
    return directory.entryList(QStringList() << "*.txt", QDir::Files);
}

bool FileHelper::removeTxtFile(const QString &fileName)
{
    QDir dir(QCoreApplication::applicationDirPath());
    return dir.remove(fileName);
}

QString FileHelper::saveTxtFile(const QString &content)
{
    QString fileName = QCoreApplication::applicationDirPath() + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt";
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
