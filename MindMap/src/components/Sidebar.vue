<template>
    <div class="MainPage">
    <aside class="Sidebar" :class="{ collapsed: leftIsCollapsed }" @mouseenter="leftIsCollapsed = false" @mouseleave="leftIsCollapsed = true">
        <h1 class="Sidebar-title">{{ msg }}</h1>
        <ul>
            <li v-for="item in navItems" :key="item.id" class="Sidebar-item">{{ item.text }}</li>
        </ul>
    </aside>

    <main class="MindMap">
        <MindMapCanvas ref="mindMapRef" @nodeSelected="onNodeSelected" />
    </main>

    <aside class="RightSidebar" :class="{ collapsed: rightIsCollapsed }" @mouseenter="rightIsCollapsed = false" @mouseleave="rightIsCollapsed = true">
        <div v-if="selectedNode" class="node-panel">
            <h1>节点操作</h1>
            <div class="node-info">
                <h3>{{ selectedNode.label }}</h3>
                <p>ID: {{ selectedNode.id }}</p>
            </div>
            <div class="actions">
                <button class="action-btn" @click="editNode">编辑</button>
                <button class="action-btn" @click="addChildNode">添加子节点</button>
                <button class="action-btn danger" @click="deleteNode">删除</button>
            </div>
        </div>
        <div v-else class="no-selection">
            <h1>右侧边栏</h1>
            <p>点击节点查看操作选项</p>
        </div>
    </aside>

    
    </div>
</template>

<script>
    import MindMapCanvas from './MindMapCanvas.vue'
    export default {
        components: {
            MindMapCanvas
        },
        data() {
            return {
                msg: 'Hello',
                navItems: [
                    { id: 1, text: '仪表盘', icon: 'fas fa-tachometer-alt' },
                    { id: 2, text: '用户管理', icon: 'fas fa-users' },
                    { id: 3, text: '数据分析', icon: 'fas fa-chart-line' },
                    { id: 4, text: '消息中心', icon: 'fas fa-envelope' },
                    { id: 5, text: '文件管理', icon: 'fas fa-folder' },
                    { id: 6, text: '系统设置', icon: 'fas fa-cog' },
                    { id: 7, text: '帮助文档', icon: 'fas fa-question-circle' }
                ],
                leftIsCollapsed: false,
                rightIsCollapsed: false,
                selectedNode: null
            }
        },
        methods: {
            onNodeSelected(node) {
                this.selectedNode = node
            },
            editNode() {
                if (this.selectedNode) {
                    this.$refs.mindMapRef.startEditNode(this.selectedNode.id)
                }
            },
            addChildNode() {
                if (this.selectedNode) {
                    this.$refs.mindMapRef.startAddChildNode(this.selectedNode.id)
                }
            },
            deleteNode() {
                if (this.selectedNode) {
                    if (confirm(`确定要删除节点 "${this.selectedNode.label}" 及其所有子节点吗？`)) {
                        this.$refs.mindMapRef.deleteSelectedNode(this.selectedNode.id)
                    }
                }
            }
        }
    }
</script>

<style scoped>
* {
    margin: 0;
    padding: 0;
}

.MainPage {
  
  display: flex;

}

.Sidebar {
  margin: 0;
  padding: 0;
  width: 200px;
  height: 100vh;
  background-color: #acb0b3;
  transition: all 0.3s ease;
  overflow: hidden;
}

.Sidebar-item {
  padding: 5px 16px;
  border-bottom: 1px solid #eee;
  cursor: pointer;
  height: 20px;
  overflow: hidden;
}

.Sidebar-item:hover {
  background-color: #57595b;
  color: white;
}


.Sidebar-title {
  font-size: 1.5em;
  margin: 0;
  padding: 0;
  background-color: #57595b;
  color: white;
  overflow: hidden;
}

.Sidebar.collapsed {
  width: 50px;
}

.MindMap {
  flex: 1;
  height: 100vh;
  background-color: #f0f0f0;
  display: flex;
  justify-content: center;
  align-items: center;
  width: calc(100% - 400px);
}

.RightSidebar {
  width: 200px;
  height: 100vh;
  background-color: #acb0b3;
  transition: all 0.3s ease;
  overflow: hidden;
  padding-left: 0.5rem;
  margin-left: 0.5rem;
  float: right;
  box-shadow: inset 5px 0 5px -5px #29627e;
  font-style: italic;
  color: #29627e;
}

.RightSidebar.collapsed{
    width: 50px;
}

ul {
    margin: 0;
    padding: 0;
    list-style-type: none;
}

/* 节点操作面板样式 */
.node-panel {
    padding: 16px;
    height: 100%;
    display: flex;
    flex-direction: column;
}

.node-panel h1 {
    font-size: 1.2em;
    margin-bottom: 16px;
    color: #29627e;
    border-bottom: 2px solid #29627e;
    padding-bottom: 8px;
}

.node-info {
    background: rgba(41, 98, 126, 0.1);
    padding: 12px;
    border-radius: 6px;
    margin-bottom: 20px;
    border: 1px solid rgba(41, 98, 126, 0.2);
}

.node-info h3 {
    margin: 0 0 8px 0;
    font-size: 1.1em;
    color: #29627e;
    font-weight: 600;
}

.node-info p {
    margin: 0;
    font-size: 0.9em;
    color: #666;
}

.actions {
    display: flex;
    flex-direction: column;
    gap: 10px;
}

.action-btn {
    padding: 10px 16px;
    border: 1px solid #acb0b3;
    background: #ffffff;
    border-radius: 6px;
    cursor: pointer;
    font-size: 14px;
    color: #29627e;
    font-weight: 500;
    transition: all 0.2s ease;
    text-align: left;
}

.action-btn:hover {
    background: #f0f8ff;
    border-color: #29627e;
    transform: translateY(-1px);
    box-shadow: 0 2px 4px rgba(41, 98, 126, 0.2);
}

.action-btn.danger {
    color: #dc2626;
    border-color: #fecaca;
}

.action-btn.danger:hover {
    background: #fef2f2;
    border-color: #fca5a5;
    box-shadow: 0 2px 4px rgba(220, 38, 38, 0.2);
}

.no-selection {
    padding: 16px;
    text-align: center;
    color: #666;
}

.no-selection h1 {
    font-size: 1.2em;
    margin-bottom: 12px;
    color: #29627e;
}

.no-selection p {
    font-size: 0.9em;
    line-height: 1.4;
}

</style>