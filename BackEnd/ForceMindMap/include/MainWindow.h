#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QWebChannel>
#include "WebNodeManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setupWebView();
    void setupToolbar();
    void loadGraph(const QJsonObject& graph);
private:
    QWebEngineView *m_webView;
    QWebChannel *m_webChannel;
    WebNodeManager *m_nodeManager;
};

#endif // MAINWINDOW_H