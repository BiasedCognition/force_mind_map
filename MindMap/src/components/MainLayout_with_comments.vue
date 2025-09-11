<template>
  <div class="MainPage">
    <!-- 左侧边栏：包含导航菜单 -->
    <!-- Vue的动态class绑定：根据leftIsCollapsed的值添加collapsed类 -->
    <!-- Vue的事件监听：@mouseenter和@mouseleave是Vue的事件修饰符 -->
    <aside class="Sidebar" :class="{ collapsed: leftIsCollapsed }" @mouseenter="leftIsCollapsed = false" @mouseleave="leftIsCollapsed = true">
      <h1 class="Sidebar-title">{{ msg }}</h1>
      <!-- Vue的列表渲染：v-for遍历navItems数组，:key为每个元素提供唯一标识符 -->
      <ul>
        <li v-for="item in navItems" :key="item.id" class="Sidebar-item">{{ item.text }}</li>
      </ul>
    </aside>

    <!-- 主内容区域：显示思维导图 -->
    <!-- Vue的组件引用：通过ref="mindMapRef"创建组件引用 -->
    <!-- Vue的事件监听：@nodeSelected监听子组件触发的自定义事件 -->
    <main class="MindMap">
      <MindMapCanvas ref="mindMapRef" @nodeSelected="onNodeSelected" />
    </main>

    <!-- 右侧边栏：节点操作面板 -->
    <!-- Vue的条件渲染：v-if根据selectedNode是否存在显示不同内容 -->
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

<script setup lang="ts">
// Vue 3 Composition API：导入所需的响应式API
import { ref, reactive } from 'vue'
// 导入子组件
import MindMapCanvas from './MindMapCanvas.vue'

// TypeScript接口定义：导航项类型
interface NavItem {
  id: number
  text: string
  icon?: string
}

// TypeScript接口定义：节点类型
interface Node {
  id: number
  x: number
  y: number
  label: string
  parentId?: number
}

// Vue的响应式引用：使用ref创建组件引用，用于访问子组件的方法
const mindMapRef = ref()
// Vue的响应式数据：使用ref创建基本类型的响应式数据
const msg = 'Hello'
// Vue的响应式数据：使用reactive创建对象/数组类型的响应式数据
const navItems = reactive<NavItem[]>([
  { id: 1, text: '仪表盘', icon: 'fas fa-tachometer-alt' },
  { id: 2, text: '用户管理', icon: 'fas fa-users' },
  { id: 3, text: '数据分析', icon: 'fas fa-chart-line' },
  { id: 4, text: '消息中心', icon: 'fas fa-envelope' },
  { id: 5, text: '文件管理', icon: 'fas fa-folder' },
  { id: 6, text: '系统设置', icon: 'fas fa-cog' },
  { id: 7, text: '帮助文档', icon: 'fas fa-question-circle' }
])

// 侧边栏折叠状态的响应式数据
const leftIsCollapsed = ref(false)
const rightIsCollapsed = ref(false)
const selectedNode = ref<Node | null>(null)

// 事件处理函数：处理子组件触发的节点选中事件
function onNodeSelected(node: Node | null) {
  selectedNode.value = node
}

// 节点编辑函数：调用子组件的startEditNode方法
function editNode() {
  if (selectedNode.value && mindMapRef.value) {
    mindMapRef.value.startEditNode(selectedNode.value.id)
  }
}

// 添加子节点函数：调用子组件的startAddChildNode方法
function addChildNode() {
  if (selectedNode.value && mindMapRef.value) {
    mindMapRef.value.startAddChildNode(selectedNode.value.id)
  }
}

// 删除节点函数：调用子组件的deleteSelectedNode方法，包含确认对话框
function deleteNode() {
  if (selectedNode.value && mindMapRef.value) {
    if (confirm(`确定要删除节点 "${selectedNode.value.label}" 及其所有子节点吗？`)) {
      mindMapRef.value.deleteSelectedNode(selectedNode.value.id)
    }
  }
}
</script>

<style scoped>
/* scoped样式：只应用于当前组件 */
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

/* 折叠状态的侧边栏 */
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

.RightSidebar.collapsed {
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