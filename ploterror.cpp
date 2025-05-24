#include "ploterror.h"
#include <QDebug>

PlotError::PlotError(ErrorType type, const QString &message)
    : m_type(type), m_message(message)
{
}

PlotError::ErrorType PlotError::type() const
{
    return m_type;
}

QString PlotError::message() const
{
    return m_message;
}

QString PlotError::typeString() const
{
    switch (m_type) {
    case KnownError:
        return QStringLiteral("已知错误");
    case UnknownError:
        return QStringLiteral("未知错误");
    default:
        return QStringLiteral("未定义错误类型");
    }
}

void PlotError::showErrorDialog(QWidget *parent, const PlotError &error)
{
    QMessageBox msgBox(parent);
    if (error.type() == KnownError) {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(QStringLiteral("已知错误"));
    } else {
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(QStringLiteral("未知错误"));
    }
    msgBox.setText(error.message());
    msgBox.exec();
}

void PlotError::debugError(const PlotError &error)
{
    qDebug() << "[" << error.typeString() << "]" << error.message();
}
