#include "mainwindow.h"

#include <QApplication>

MainWindow *g_mainWindow = nullptr; // 全局主窗口指针

int main(int argc, char *argv[])
{
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    MainWindow w;
    g_mainWindow = &w; // 赋值全局指针
    w.show();
    return a.exec();
}
