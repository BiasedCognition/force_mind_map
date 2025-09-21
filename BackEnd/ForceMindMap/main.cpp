#include "MainWindow.h"
#include <QApplication>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QDebug>
#include <QDir>

int main(int argc, char *argv[])
{
    // 启用WebEngine远程调试
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "8080");
    
    QApplication app(argc, argv);

    // 设置应用程序元数据
    app.setApplicationName("ForceMindMap");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("YourOrganization");
    
    // 输出调试信息
    qDebug() << "WebEngine remote debugging enabled at http://localhost:8080";

    // 创建并显示主窗口
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}