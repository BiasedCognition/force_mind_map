#include <QApplication>
#include <QWebEngineView>
#include <QWebChannel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

class WebBridge : public QObject {
    Q_OBJECT
public slots:
    void cppFunction() {
        qDebug() << "C++ function called from JavaScript!";
        QMessageBox::information(nullptr, "Success", "C++ function called from JavaScript!");
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 创建主窗口和布局
    QWidget window;
    QVBoxLayout layout(&window);
    window.setWindowTitle("QWebEngine Test");
    window.resize(1024, 768);

    // 创建 WebEngineView 和 WebChannel
    QWebEngineView view;
    QWebChannel channel;
    WebBridge bridge;
    
    // 注册对象到WebChannel
    channel.registerObject(QStringLiteral("bridge"), &bridge);
    
    // 设置WebChannel到页面
    if (view.page()) {
        view.page()->setWebChannel(&channel);
        qDebug() << "WebChannel set up successfully";
    } else {
        qDebug() << "Error: WebEngineView page is nullptr";
    }

    // 创建测试HTML字符串
    QString html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="UTF-8">
            <title>QWebEngine Test</title>
            <script src="qrc:///qtwebchannel/qwebchannel.js"></script>
            <style>
                body {
                    font-family: Arial, sans-serif;
                    padding: 20px;
                    text-align: center;
                }
                button {
                    padding: 10px 20px;
                    font-size: 16px;
                    margin: 10px;
                }
            </style>
        </head>
        <body>
            <h1>QWebEngineView & QWebChannel Test</h1>
            <p>Testing basic communication between Qt C++ and JavaScript</p>
            <button id="callCppBtn">Call C++ Function</button>
            
            <script>
                // 等待文档加载完成
                document.addEventListener('DOMContentLoaded', function() {
                    console.log('Document loaded');
                    
                    // 初始化WebChannel
                    if (typeof QWebChannel !== 'undefined') {
                        new QWebChannel(qt.webChannelTransport, function(channel) {
                            console.log('WebChannel connected');
                            
                            // 获取bridge对象
                            window.bridge = channel.objects.bridge;
                            console.log('Bridge object obtained:', window.bridge);
                             
                            // 添加按钮点击事件
                            document.getElementById('callCppBtn').addEventListener('click', function() {
                                if (window.bridge && typeof window.bridge.cppFunction === 'function') {
                                    console.log('Calling C++ function...');
                                    window.bridge.cppFunction();
                                } else {
                                    console.error('Bridge or cppFunction not available');
                                    alert('Error: Cannot communicate with C++ backend');
                                }
                            });
                        });
                    } else {
                        console.error('QWebChannel is not defined!');
                        alert('Error: QWebChannel not available');
                    }
                });
            </script>
        </body>
        </html>
    )";

    // 保存测试HTML到临时文件
    QString tempHtmlPath = QCoreApplication::applicationDirPath() + "/test.html";
    {
        QFile file(tempHtmlPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << html;
            file.close();
            qDebug() << "Test HTML saved to:" << tempHtmlPath;
        } else {
            qDebug() << "Error: Cannot save test HTML file";
        }
    }

    // 加载测试HTML文件
    view.load(QUrl::fromLocalFile(tempHtmlPath));

    // 添加控制按钮
    QPushButton backButton("Back");
    QPushButton forwardButton("Forward");
    QPushButton reloadButton("Reload");
    
    layout.addWidget(&backButton);
    layout.addWidget(&forwardButton);
    layout.addWidget(&reloadButton);
    layout.addWidget(&view);

    // 信号槽连接
    QObject::connect(&backButton, &QPushButton::clicked, &view, &QWebEngineView::back);
    QObject::connect(&forwardButton, &QPushButton::clicked, &view, &QWebEngineView::forward);
    QObject::connect(&reloadButton, &QPushButton::clicked, &view, &QWebEngineView::reload);

    // 显示窗口
    window.show();

    return app.exec();
}

#include "WebEngineTest.moc"