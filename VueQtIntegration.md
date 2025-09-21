# Vue 前端与 Qt 后端集成方案

## 概述

本方案详细说明如何将 MindMap 文件夹的 Vue 前端与 BackEnd\ForceMindMap 文件夹的 Qt 后端集成，利用 QWebEngine 和 QWebChannel 实现前后端通信。

## 项目结构

```
├── BackEnd\
│   └── ForceMindMap\  # Qt C++ 后端项目
├── MindMap\
│   └── MindMap\       # Vue 前端项目
```

## 技术原理

1. **Qt 后端**：使用 QWebEngineView 加载 Vue 前端构建的 HTML 文件
2. **通信机制**：通过 QWebChannel 实现 Qt C++ 与 JavaScript 的双向通信
3. **数据交换**：使用 JSON 格式在前后端之间传递数据

## 实现步骤

### 1. 修改 Qt 项目

#### 1.1 修改 MainWindow.cpp 文件

```cpp
void MainWindow::setupWebView()
{
    // 设置 WebChannel
    m_webChannel->registerObject("nodeManager", m_nodeManager);
    m_webView->page()->setWebChannel(m_webChannel);

    // 启用必要的 WebEngine 设置
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);

    // 加载 Vue 前端构建的 HTML 文件
    // 注意：这里需要先构建 Vue 项目，然后将构建后的文件路径更新为实际路径
    QUrl url = QUrl::fromLocalFile("D:/MindMap/MindMap/MindMap/dist/index.html");
    m_webView->load(url);

    setCentralWidget(m_webView);
}
```

#### 1.2 扩展 WebNodeManager 类以支持更多操作

根据 Vue 前端的需求，扩展 WebNodeManager 类以支持更多的思维导图操作。

### 2. 配置 Vue 项目

#### 2.1 修改 vite.config.ts 以支持 Qt WebEngine

```ts
import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import vueDevTools from 'vite-plugin-vue-devtools'
import { fileURLToPath } from 'url'
import path from 'path'

export default defineConfig({
  plugins: [
    vue(),
    vueDevTools(),
  ],
  base: './', // 确保资源路径使用相对路径
  build: {
    outDir: 'dist',
    assetsDir: 'assets',
  },
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url))
    },
  },
})
```

#### 2.2 在 Vue 项目中添加 qwebchannel.js

将 Qt 的 qwebchannel.js 文件复制到 Vue 项目的 public 目录中。这个文件通常可以在 Qt 安装目录中找到，路径类似于 `Qt/6.x.x/mingw_64/resources/qwebchannel/qwebchannel.js`。

### 3. 在 Vue 前端实现与 Qt 后端的通信

#### 3.1 创建通信服务类

在 Vue 项目中创建一个通信服务类，用于处理与 Qt 后端的通信。

```typescript
// src/services/qtCommunicationService.ts

export interface Node {
  id: string;
  text: string;
  parent?: string;
  x?: number;
  y?: number;
}

export interface Link {
  source: string;
  target: string;
}

export interface GraphData {
  nodes: Node[];
  links: Link[];
}

class QtCommunicationService {
  private nodeManager: any;
  private isConnected = false;
  private connectionCallback?: () => void;

  // 初始化连接
  initConnection(callback?: () => void): void {
    this.connectionCallback = callback;
    
    if (window.qt && window.qt.webChannelTransport) {
      this.setupChannel();
    } else {
      // 等待 qt 对象加载
      setTimeout(() => this.initConnection(callback), 100);
    }
  }

  // 设置通信通道
  private setupChannel(): void {
    // @ts-ignore
    new QWebChannel(qt.webChannelTransport, (channel: any) => {
      this.nodeManager = channel.objects.nodeManager;
      this.isConnected = true;
      
      // 注册事件监听器
      this.setupEventListeners();
      
      // 调用回调函数
      if (this.connectionCallback) {
        this.connectionCallback();
      }
    });
  }

  // 设置事件监听器
  private setupEventListeners(): void {
    if (!this.isConnected) return;
    
    // 监听节点添加事件
    this.nodeManager.nodeAdded.connect((node: Node) => {
      console.log('Node added:', node);
      // 触发 Vue 组件中的事件
    });
    
    // 监听节点删除事件
    this.nodeManager.nodeRemoved.connect((id: string) => {
      console.log('Node removed:', id);
      // 触发 Vue 组件中的事件
    });
    
    // 监听节点更新事件
    this.nodeManager.nodeUpdated.connect((node: Node) => {
      console.log('Node updated:', node);
      // 触发 Vue 组件中的事件
    });
    
    // 监听图更新事件
    this.nodeManager.graphUpdated.connect((graph: GraphData) => {
      console.log('Graph updated:', graph);
      // 触发 Vue 组件中的事件
    });
  }

  // 添加节点
  addNode(id: string, text: string, parentId: string = ''): void {
    if (!this.isConnected) return;
    this.nodeManager.addNode(id, text, parentId);
  }

  // 删除节点
  removeNode(id: string): void {
    if (!this.isConnected) return;
    this.nodeManager.removeNode(id);
  }

  // 更新节点文本
  updateNodeText(id: string, text: string): void {
    if (!this.isConnected) return;
    this.nodeManager.updateNodeText(id, text);
  }

  // 添加连接
  addConnection(sourceId: string, targetId: string): void {
    if (!this.isConnected) return;
    this.nodeManager.addConnection(sourceId, targetId);
  }

  // 删除连接
  removeConnection(sourceId: string, targetId: string): void {
    if (!this.isConnected) return;
    this.nodeManager.removeConnection(sourceId, targetId);
  }

  // 获取完整图数据
  async getFullGraph(): Promise<GraphData> {
    if (!this.isConnected) return { nodes: [], links: [] };
    
    // 使用 Promise 包装同步调用
    return new Promise((resolve) => {
      const graph = this.nodeManager.getFullGraph();
      resolve(graph);
    });
  }
}

export default new QtCommunicationService();
```

#### 3.2 修改 index.html 引入 qwebchannel.js

```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <link rel="icon" href="/favicon.ico">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Mind Map</title>
  <!-- 引入 qwebchannel.js -->
  <script src="/qwebchannel.js"></script>
</head>
<body>
  <div id="app"></div>
  <script type="module" src="/src/main.ts"></script>
</body>
</html>
```

#### 3.3 在 Vue 组件中使用通信服务

修改 MindMapCanvas.vue 组件，使其能够与 Qt 后端进行通信。

### 4. 构建和集成

#### 4.1 构建 Vue 前端

```bash
cd MindMap/MindMap
npm install
npm run build
```

这将在 `dist` 目录中生成构建后的文件。

#### 4.2 修改 Qt 项目的资源文件

更新 Qt 项目的 CMakeLists.txt 文件，将 Vue 构建后的文件添加到 Qt 资源系统中：

```cmake
qt_add_resources(${PROJECT_NAME} "vue_resources"
        PREFIX "/"
        FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../MindMap/MindMap/dist/index.html"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../MindMap/MindMap/dist/assets/"
)
```

#### 4.3 修改 MainWindow.cpp 加载资源

```cpp
void MainWindow::setupWebView()
{
    // 设置 WebChannel
    m_webChannel->registerObject("nodeManager", m_nodeManager);
    m_webView->page()->setWebChannel(m_webChannel);

    // 启用必要的 WebEngine 设置
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);

    // 加载 Vue 前端构建的 HTML 文件（从资源中加载）
    m_webView->setHtml("<html><body>Loading...</body></html>");
    QFile htmlFile("qrc:/index.html");
    if (htmlFile.open(QIODevice::ReadOnly)) {
        QString htmlContent = htmlFile.readAll();
        m_webView->setHtml(htmlContent, QUrl("qrc:/"));
    } else {
        m_webView->setHtml("<h1>Error loading Vue app</h1>");
        qWarning() << "Failed to load Vue app resource:" << htmlFile.errorString();
    }

    setCentralWidget(m_webView);
}
```

## 测试和调试

1. 确保 Qt 项目能够正确加载 Vue 前端
2. 在 Vue 前端中添加调试日志，验证与 Qt 后端的通信
3. 使用 Qt 的调试工具和浏览器的开发者工具进行联合调试

## 注意事项

1. **版本兼容性**：确保 Qt 和 Vue 版本的兼容性
2. **资源路径**：注意处理相对路径和绝对路径的问题
3. **性能优化**：对于大型思维导图，考虑分批加载数据以提高性能
4. **错误处理**：添加适当的错误处理和重试机制
5. **安全性**：注意处理来自前端的输入验证，防止注入攻击

## 扩展功能建议

1. 添加离线支持
2. 实现数据导出/导入功能
3. 添加用户认证和权限控制
4. 实现协作编辑功能
5. 添加主题切换功能