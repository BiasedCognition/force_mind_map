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
  private eventListeners: Map<string, Set<Function>> = new Map();
  // 生产环境下禁用详细日志
  private readonly DEBUG_MODE = false;

  // 初始化连接
  initialize(callback?: () => void): void {
    this.connectionCallback = callback;
    
    // @ts-ignore - qt is provided by the Qt WebEngine environment
    if (window.qt && window.qt.webChannelTransport) {
      this.setupChannel();
    } else {
      // 等待 qt 对象加载
      setTimeout(() => this.initialize(callback), 100);
    }
  }
  
  // 为了向后兼容，保留initConnection方法
  initConnection(callback?: () => void): void {
    this.initialize(callback);
  }

  // 设置通信通道
  private setupChannel(): void {
    // @ts-ignore
    new QWebChannel(qt.webChannelTransport, (channel: any) => {
      this.nodeManager = channel.objects.nodeManager;
      this.isConnected = true;
      
      console.log('Connected to Qt backend');
      
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
      if (this.DEBUG_MODE) console.log('Node added:', node);
      this.emit('nodeAdded', node);
    });
    
    // 监听节点删除事件
    this.nodeManager.nodeRemoved.connect((id: string) => {
      if (this.DEBUG_MODE) console.log('Node removed:', id);
      this.emit('nodeRemoved', id);
    });
    
    // 监听节点更新事件
    this.nodeManager.nodeUpdated.connect((node: Node) => {
      if (this.DEBUG_MODE) console.log('Node updated:', node);
      this.emit('nodeUpdated', node);
    });
    
    // 监听连接添加事件
    this.nodeManager.connectionAdded.connect((link: Link) => {
      if (this.DEBUG_MODE) console.log('Connection added:', link);
      this.emit('connectionAdded', link);
    });
    
    // 监听连接删除事件
    this.nodeManager.connectionRemoved.connect((sourceId: string, targetId: string) => {
      if (this.DEBUG_MODE) console.log('Connection removed:', sourceId, targetId);
      this.emit('connectionRemoved', sourceId, targetId);
    });
    
    // 监听图更新事件
    this.nodeManager.graphUpdated.connect((graph: GraphData) => {
      if (this.DEBUG_MODE) console.log('Graph updated:', graph);
      this.emit('graphUpdated', graph);
    });
  }

  // 事件订阅
  on(event: string, callback: Function): void {
    if (!this.eventListeners.has(event)) {
      this.eventListeners.set(event, new Set());
    }
    this.eventListeners.get(event)?.add(callback);
  }

  // 取消事件订阅
  off(event: string, callback: Function): void {
    this.eventListeners.get(event)?.delete(callback);
  }

  // 触发事件
  private emit(event: string, ...args: any[]): void {
    this.eventListeners.get(event)?.forEach(callback => {
      try {
        callback(...args);
      } catch (error) {
        console.error('Error in event listener:', error);
      }
    });
  }

  // 添加节点
  addNode(id: string, text: string, parentId: string = ''): void {
    if (!this.isConnected) {
      console.warn('Not connected to Qt backend');
      return;
    }
    console.log('Adding node:', id, text, parentId);
    this.nodeManager.addNode(id, text, parentId);
  }

  // 删除节点
  removeNode(id: string): void {
    if (!this.isConnected) {
      console.warn('Not connected to Qt backend');
      return;
    }
    console.log('Removing node:', id);
    this.nodeManager.removeNode(id);
  }

  // 更新节点
  updateNode(id: string, text: string): void {
    if (!this.isConnected) {
      console.warn('Not connected to Qt backend');
      return;
    }
    console.log('Updating node:', id, text);
    // 如果nodeManager有updateNode方法，使用它；否则回退到updateNodeText
    if (this.nodeManager.updateNode) {
      this.nodeManager.updateNode(id, text);
    } else if (this.nodeManager.updateNodeText) {
      this.nodeManager.updateNodeText(id, text);
    }
  }
  
  // 更新节点位置
  updateNodePosition(id: string, x: number, y: number): void {
    if (!this.isConnected) {
      console.warn('Not connected to Qt backend');
      return;
    }
    console.log('Updating node position:', id, x, y);
    if (this.nodeManager.updateNodePosition) {
      this.nodeManager.updateNodePosition(id, x, y);
    } else {
      console.warn('nodeManager does not support updateNodePosition method');
    }
  }
  
  // 更新完整图数据
  updateFullGraph(graph: GraphData): void {
    if (!this.isConnected) {
      console.warn('Not connected to Qt backend');
      return;
    }
    console.log('Updating full graph');
    if (this.nodeManager.updateFullGraph) {
      this.nodeManager.updateFullGraph(graph);
    } else {
      console.warn('nodeManager does not support updateFullGraph method');
      // 备选方案：如果没有updateFullGraph方法，手动更新每个节点和连接
      try {
        // 先清空所有节点和连接
        // 注意：这只是一个备选方案，在实际应用中可能需要更复杂的同步逻辑
        // this.nodeManager.clearGraph();
        
        // 然后添加所有节点
        graph.nodes.forEach(node => {
          this.addNode(node.id, node.text, node.parent || '');
          if (node.x !== undefined && node.y !== undefined) {
            this.updateNodePosition(node.id, node.x, node.y);
          }
        });
        
        // 然后添加所有连接
        graph.links.forEach(link => {
          this.addConnection(link.source, link.target);
        });
      } catch (error) {
        console.error('Error updating graph manually:', error);
      }
    }
  }
  
  // 为了向后兼容，保留updateNodeText方法
  updateNodeText(id: string, text: string): void {
    this.updateNode(id, text);
  }

  // 添加连接
  addConnection(sourceId: string, targetId: string): void {
    if (!this.isConnected) {
      console.warn('Not connected to Qt backend');
      return;
    }
    console.log('Adding connection:', sourceId, targetId);
    this.nodeManager.addConnection(sourceId, targetId);
  }

  // 删除连接
  removeConnection(sourceId: string, targetId: string): void {
    if (!this.isConnected) {
      console.warn('Not connected to Qt backend');
      return;
    }
    console.log('Removing connection:', sourceId, targetId);
    this.nodeManager.removeConnection(sourceId, targetId);
  }

  // 获取完整图数据
  async getFullGraph(): Promise<GraphData> {
    if (!this.isConnected) {
      console.warn('Not connected to Qt backend');
      return { nodes: [], links: [] };
    }
    
    try {
      console.log('Getting full graph');
      const graph = this.nodeManager.getFullGraph();
      return graph;
    } catch (error) {
      console.error('Error getting full graph:', error);
      return { nodes: [], links: [] };
    }
  }

  // 检查连接状态
  getIsConnected(): boolean {
    return this.isConnected;
  }
}

export default new QtCommunicationService();