<template>
  <aside class="RightSidebar" :class="{ collapsed: isCollapsed }" @mouseenter="isCollapsed = false" @mouseleave="isCollapsed = true">
    <!-- 视图切换按钮 -->
    <div class="view-toggle">
      <button class="toggle-btn" @click="handleToggleView">
        {{ showMindMap ? '📝 切换到编辑器' : '🗺️ 切换到思维导图' }}
      </button>
    </div>

    <!-- 节点操作面板 -->
    <div v-if="selectedNode && showMindMap" class="node-panel">
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
    
    <!-- 无选中状态或显示Markdown编辑器时 -->
    <div v-else class="no-selection">
      <h1>右侧边栏</h1>
      <p>{{ showMindMap ? '点击节点查看操作选项' : '当前为Markdown编辑器模式' }}</p>
    </div>
  </aside>
</template>

<script setup lang="ts">
import { ref } from 'vue'

interface Node {
  id: number
  x: number
  y: number
  label: string
  parentId?: number
}

// 定义事件
const emit = defineEmits<{
  'toggle-view': []
}>()

// 接收props
const props = defineProps<{
  selectedNode: Node | null
  mindMapRef: any
  showMindMap: boolean
}>()

// 侧边栏折叠状态
const isCollapsed = ref(false)

// 处理视图切换
function handleToggleView() {
  emit('toggle-view')
}

// 编辑节点
function editNode() {
  if (props.selectedNode && props.mindMapRef && props.showMindMap) {
    props.mindMapRef.startEditNode(props.selectedNode.id)
  }
}

// 添加子节点
function addChildNode() {
  if (props.selectedNode && props.mindMapRef && props.showMindMap) {
    props.mindMapRef.startAddChildNode(props.selectedNode.id)
  }
}

// 删除节点
function deleteNode() {
  if (props.selectedNode && props.mindMapRef && props.showMindMap) {
    if (confirm(`确定要删除节点 "${props.selectedNode.label}" 及其所有子节点吗？`)) {
      props.mindMapRef.deleteSelectedNode(props.selectedNode.id)
    }
  }
}
</script>

<style scoped>
.RightSidebar {
  width: 200px;
  height: 100vh;
  background-color: #acb0b3;
  transition: all 0.3s ease;
  overflow-y: auto;
  padding-left: 0.5rem;
  margin-left: 0.5rem;
  float: right;
  box-shadow: inset 5px 0 5px -5px #29627e;
  font-style: italic;
  color: #29627e;
}

/* 视图切换按钮样式 */
.view-toggle {
  padding: 16px;
  border-bottom: 2px solid #29627e;
  margin-bottom: 16px;
  background: rgba(41, 98, 126, 0.1);
  overflow: hidden;
}

.toggle-btn {
  width: 100%;
  padding: 10px;
  border: none;
  border-radius: 6px;
  background-color: #29627e;
  color: white;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: background-color 0.2s ease;
  overflow: hidden;
  white-space: nowrap;
  text-overflow: ellipsis;
}

.toggle-btn:hover {
  background-color: #1f4b61;
}

.toggle-btn:active {
  transform: scale(0.98);
}

.RightSidebar.collapsed {
  width: 50px;
}

/* 节点操作面板样式 */
.node-panel {
  padding: 16px;
  height: 100%;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}

.node-panel h1 {
  font-size: 1.2em;
  margin-bottom: 16px;
  color: #29627e;
  border-bottom: 2px solid #29627e;
  padding-bottom: 8px;
  overflow: hidden;
  white-space: nowrap;
  text-overflow: ellipsis;
}

.node-info {
  background: rgba(41, 98, 126, 0.1);
  padding: 12px;
  border-radius: 6px;
  margin-bottom: 20px;
  border: 1px solid rgba(41, 98, 126, 0.2);
  overflow: hidden;
}

.node-info h3 {
  margin: 0 0 8px 0;
  font-size: 1.1em;
  color: #29627e;
  font-weight: 600;
  overflow: hidden;
  white-space: nowrap;
  text-overflow: ellipsis;
}

.node-info p {
  margin: 0;
  font-size: 0.9em;
  color: #666;
  overflow: hidden;
  white-space: nowrap;
  text-overflow: ellipsis;
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
  overflow: hidden;
  white-space: nowrap;
  text-overflow: ellipsis;
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
  overflow: hidden;
}

.no-selection h1 {
  font-size: 1.2em;
  margin-bottom: 12px;
  color: #29627e;
  overflow: hidden;
  white-space: nowrap;
  text-overflow: ellipsis;
}

.no-selection p {
  font-size: 0.9em;
  line-height: 1.4;
  overflow: hidden;
  white-space: nowrap;
  text-overflow: ellipsis;
}
</style>