#ifndef PLOTERROR_H
#define PLOTERROR_H
#include <QString>
#include <QMessageBox>

#define ERR 0//错误
#define OK 1//正常


class PlotError
{
public:
    enum ErrorType {
        KnownError,
        UnknownError
    };

    PlotError(ErrorType type, const QString &message);

    ErrorType type() const;
    QString message() const;
    QString typeString() const;
    // 静态方法：弹窗提示错误
    static void showErrorDialog(QWidget *parent, const PlotError &error);
    // 静态方法：QDebug调试输出错误
    static void debugError(const PlotError &error);

private:
    ErrorType m_type;
    QString m_message;
};

#endif // PLOTERROR_H
