#include <QApplication>
#include <QDebug>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== Qt 多平台行事曆整合工具 ===";
    qDebug() << "支援 Google Calendar 和 Microsoft Outlook 整合";
    qDebug() << "";
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
