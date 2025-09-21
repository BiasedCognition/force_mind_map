// src/types/qt.d.ts

// 声明全局的 qt 对象
interface QtWebChannelTransport {
  send(data: any): void;
  onmessage?: ((event: { data: any }) => void) | null;
}

declare namespace qt {
  const webChannelTransport: QtWebChannelTransport;
}

// 声明 QWebChannel 类
interface QWebChannel {
  new(transport: QtWebChannelTransport, callback: (channel: { objects: Record<string, any> }) => void): void;
}

// 添加到全局 window 对象
interface Window {
  qt: typeof qt;
  QWebChannel: QWebChannel;
}