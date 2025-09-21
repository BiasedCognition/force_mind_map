<template>
  <aside 
    class="transition-all duration-300 overflow-y-auto pl-2 ml-2 float-right shadow-inner shadow-blue-800/30 italic text-blue-800 bg-gray-400 h-screen"
    :class="isCollapsed ? 'w-12' : 'w-50'"
    @mouseenter="isCollapsed = false" 
    @mouseleave="isCollapsed = true">
    <!-- 视图切换按钮 -->
    <div class="p-4 border-b-2 border-blue-800 mb-4 bg-blue-800/10 overflow-hidden">
      <button 
        class="w-full py-2.5 px-0 border-none rounded-md bg-blue-800 text-white text-sm font-medium cursor-pointer transition-all hover:bg-blue-900 active:scale-98 overflow-hidden whitespace-nowrap text-ellipsis"
        @click="handleToggleView">
        {{ showMindMap ? '📝 切换到编辑器' : '🗺️ 切换到思维导图' }}
      </button>
    </div>

    <!-- 节点操作面板 -->
    <div v-if="selectedNode && showMindMap" class="p-4 h-full flex flex-col overflow-hidden">
      <h1 class="text-lg mb-4 text-blue-800 border-b-2 border-blue-800 pb-2 overflow-hidden whitespace-nowrap text-ellipsis">节点操作</h1>
      <div class="bg-blue-800/10 p-3 rounded-md mb-5 border border-blue-800/20 overflow-hidden">
        <h3 class="mb-2 text-lg text-blue-800 font-semibold overflow-hidden whitespace-nowrap text-ellipsis">{{ selectedNode.label }}</h3>
        <p class="text-sm text-gray-600 overflow-hidden whitespace-nowrap text-ellipsis">ID: {{ selectedNode.id }}</p>
      </div>
      <div class="flex flex-col gap-2.5">
        <button class="py-2.5 px-4 border border-gray-400 bg-white rounded-md cursor-pointer text-sm text-blue-800 font-medium transition-all hover:bg-blue-50 hover:border-blue-800 hover:-translate-y-0.5 hover:shadow-md text-left overflow-hidden whitespace-nowrap text-ellipsis" @click="editNode">编辑</button>
        <button class="py-2.5 px-4 border border-gray-400 bg-white rounded-md cursor-pointer text-sm text-blue-800 font-medium transition-all hover:bg-blue-50 hover:border-blue-800 hover:-translate-y-0.5 hover:shadow-md text-left overflow-hidden whitespace-nowrap text-ellipsis" @click="addChildNode">添加子节点</button>
        <button class="py-2.5 px-4 border border-red-100 bg-white rounded-md cursor-pointer text-sm text-red-600 font-medium transition-all hover:bg-red-50 hover:border-red-300 hover:-translate-y-0.5 hover:shadow-md text-left overflow-hidden whitespace-nowrap text-ellipsis" @click="deleteNode">删除</button>
      </div>
    </div>
    
    <!-- 无选中状态或显示Markdown编辑器时 -->
    <div v-else class="p-4 text-center text-gray-600 overflow-hidden">
      <h1 class="text-lg mb-3 text-blue-800 overflow-hidden whitespace-nowrap text-ellipsis">右侧边栏</h1>
      <p class="text-sm leading-relaxed overflow-hidden whitespace-nowrap text-ellipsis">{{ showMindMap ? '点击节点查看操作选项' : '当前为Markdown编辑器模式' }}</p>
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
/* 保持一些无法用Tailwind直接替代的过渡效果 */
</style>