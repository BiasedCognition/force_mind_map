#include "MainWindow.h"
#include "AppConfig.h"
#include <QToolBar>
#include <QAction>
#include <QFile>
#include <QUrl>
#include <QWebEngineSettings>
#include <QInputDialog>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_webView(new QWebEngineView(this))
    , m_webChannel(new QWebChannel(this))
    , m_nodeManager(new WebNodeManager(this))
{
    setWindowTitle("D3.js Mind Map");
    resize(1200, 800);

    setupWebView();
    setupToolbar();

    // 添加测试数据
    m_nodeManager->addNode("1", "Root Node");
    m_nodeManager->addNode("2", "Child 1", "1");
    m_nodeManager->addNode("3", "Child 2", "1");
    m_nodeManager->addConnection("1", "2");
    m_nodeManager->addConnection("1", "3");
}

void MainWindow::setupWebView()
{
    // 设置WebChannel
    m_webChannel->registerObject("nodeManager", m_nodeManager);
    m_webView->page()->setWebChannel(m_webChannel);

    // 启用必要的设置
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    
    // 性能优化设置
    m_webView->settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::WebGLEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);
    
    // 减少不必要的渲染
    m_webView->settings()->setAttribute(QWebEngineSettings::AutoLoadImages, true);
    
    // 使用配置类获取前端URL（根据环境自动切换）
    QString frontendUrl = AppConfig::getFrontendUrl();
    
    // 输出当前加载的路径和模式，方便调试
    QString mode = AppConfig::useDevMode() ? "development mode" : "production mode";
    qDebug() << "Loading Vue frontend from:" << frontendUrl;
    qDebug() << "Application running in:" << mode;
    
    // 根据URL类型选择适当的加载方式
    if (frontendUrl.startsWith("qrc:/")) {
        // 对于Qt资源，直接使用QUrl构造函数
        m_webView->load(QUrl(frontendUrl));
    } else {
        // 对于其他URL，使用fromUserInput
        m_webView->load(QUrl::fromUserInput(frontendUrl));
    }

    setCentralWidget(m_webView);
}

void MainWindow::setupToolbar()
{
    QToolBar *toolbar = addToolBar("Tools");

    QAction *addNodeAction = toolbar->addAction("Add Node");
    connect(addNodeAction, &QAction::triggered, [this]() {
        static int counter = 0;
        QString id = QString::number(100 + counter++);
        m_nodeManager->addNode(id, "New Node " + QString::number(counter), "1");
        m_nodeManager->addConnection("1", id);
    });

    QAction *refreshAction = toolbar->addAction("Refresh");
    connect(refreshAction, &QAction::triggered, [this]() {
        m_webView->reload();
    });

    QAction *editAction = toolbar->addAction("Edit Node");
    connect(editAction, &QAction::triggered, [this]() {
        bool ok;
        QString text = QInputDialog::getText(this, "Edit Node", "Enter new text:",
                                            QLineEdit::Normal, "Node Text", &ok);
        if (ok && !text.isEmpty()) {
            // 编辑根节点
            m_nodeManager->updateNodeText("1", text);
        }
    });

    QAction *saveAction = toolbar->addAction("Save");
    connect(saveAction, &QAction::triggered, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, "Save Mind Map", "", "Mind Map Files (*.mmap)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                QJsonDocument doc(m_nodeManager->getFullGraph());
                file.write(doc.toJson());
                file.close();
            } else {
                QMessageBox::warning(this, "Save Error", "Failed to save file: " + file.errorString());
            }
        }
    });

    QAction *loadAction = toolbar->addAction("Load");
    connect(loadAction, &QAction::triggered, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Load Mind Map", "", "Mind Map Files (*.mmap)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
                loadGraph(doc.object());
                file.close();
            } else {
                QMessageBox::warning(this, "Load Error", "Failed to open file: " + file.errorString());
            }
        }
    });
}

void MainWindow::loadGraph(const QJsonObject& graph)
{
    // 清除现有数据
    m_nodeManager->removeNode("1");
    m_nodeManager->removeNode("2");
    m_nodeManager->removeNode("3");

    // 添加节点
    QJsonArray nodes = graph["nodes"].toArray();
    for (const auto& nodeVal : nodes) {
        QJsonObject node = nodeVal.toObject();
        m_nodeManager->addNode(
            node["id"].toString(),
            node["text"].toString(),
            node["parent"].toString()
        );
    }

    // 添加连接
    QJsonArray links = graph["links"].toArray();
    for (const auto& linkVal : links) {
        QJsonObject link = linkVal.toObject();
        m_nodeManager->addConnection(
            link["source"].toString(),
            link["target"].toString()
        );
    }
}