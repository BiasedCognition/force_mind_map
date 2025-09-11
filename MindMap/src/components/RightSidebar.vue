<template>
  <aside class="RightSidebar" :class="{ collapsed: isCollapsed }" @mouseenter="isCollapsed = false" @mouseleave="isCollapsed = true">
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

// 接收props
const props = defineProps<{
  selectedNode: Node | null
  mindMapRef: any
}>()

// 侧边栏折叠状态
const isCollapsed = ref(false)

// 编辑节点
function editNode() {
  if (props.selectedNode && props.mindMapRef) {
    props.mindMapRef.startEditNode(props.selectedNode.id)
  }
}

// 添加子节点
function addChildNode() {
  if (props.selectedNode && props.mindMapRef) {
    props.mindMapRef.startAddChildNode(props.selectedNode.id)
  }
}

// 删除节点
function deleteNode() {
  if (props.selectedNode && props.mindMapRef) {
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