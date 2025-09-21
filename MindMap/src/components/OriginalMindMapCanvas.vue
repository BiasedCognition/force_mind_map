<script setup lang="ts">
import { ref, reactive, computed, onMounted, onBeforeUnmount, nextTick, watch } from 'vue'

const emit = defineEmits<{
  nodeSelected: [node: Node | null]
  nodeEdit: [nodeId: number]
  nodeAddChild: [nodeId: number]
  nodeDelete: [nodeId: number]
}>()

type Node = {
  id: number
  x: number // 相对于根节点的坐标
  y: number
  label: string
  parentId?: number
}

const STORAGE_KEY = 'mindmap_v1'

const containerRef = ref<HTMLElement | null>(null)
const nodes = reactive<Node[]>([])
const nextId = ref(1)

const selectedParentId = ref<number | null>(null)
const isAdding = computed(() => selectedParentId.value !== null)
const tempPos = reactive({ x: 0, y: 0 }) // world coords

// 拖拽状态（节点）
const draggingId = ref<number | null>(null)
const dragOffset = reactive({ x: 0, y: 0 })

// 编辑状态
const editingId = ref<number | null>(null)
const editingText = ref('')
const editInput = ref<HTMLInputElement | null>(null)

// 选中状态（用于 Delete 删除）
const selectedId = ref<number | null>(null)

// 视图缩放/平移（屏幕 = translate(pan) + scale(scale) 应用于 content 容器）
const pan = reactive({ x: 0, y: 0 })
const scale = ref(1)
const minScale = 0.3
const maxScale = 2.5

// 平移拖拽
const isPanning = ref(false)
const spacePressed = ref(false)
const panStart = reactive({ x: 0, y: 0 })
const panAtStart = reactive({ x: 0, y: 0 })

// 样式自定义
const styleOpts = reactive({
  nodeBg: '#ffffff',
  nodeBorder: '#dcdfe6',
  nodeText: '#333333',
  linkColor: '#9aa4b2',
  linkWidth: 2,
  curvature: 0.30,   // 与距离成比例的曲率系数
  maxOffset: 80      // 弧度上限
})

onMounted(() => {
  window.addEventListener('keydown', onKeydown)
  window.addEventListener('keyup', onKeyup)
  
  // 加载本地数据
  loadFromLocal()
  if (nodes.length === 0) {
    const el = containerRef.value
    const rect = el?.getBoundingClientRect()
    const cx = rect ? rect.width / 2 : 400
    const cy = rect ? rect.height / 2 : 300
    const rootNode = { id: nextId.value++, x: cx, y: cy, label: 'Root' }
    nodes.push(rootNode)
    saveToLocal()
  }
  
  // 在DOM更新后，将第二个元素的位置设置为第一个元素的上半部分中心
  nextTick(() => {
    if (nodes.length >= 2) {
      const firstNode = nodes[0]
      const secondNode = nodes[1]
      
      // 计算第二个节点在上半部分中心的位置
      // 由于节点使用了transform: translate(-50%, -50%)，我们直接设置中心点位置
      // 在上半部分，Y坐标设置为-80像素（相对于第一个节点）
      secondNode.x = firstNode.x
      secondNode.y = firstNode.y - 80
    }
  })
})

// 从本地存储加载数据
function loadFromLocal() {
  try {
    const stored = localStorage.getItem(STORAGE_KEY)
    if (stored) {
      const data = JSON.parse(stored)
      if (data.nodes && Array.isArray(data.nodes)) {
        // 清空现有节点并添加新节点
        nodes.length = 0
        data.nodes.forEach((node: Node) => nodes.push({ ...node }))
        
        // 更新nextId
        nextId.value = Math.max(...nodes.map(n => n.id), 1) + 1
      }
    }
  } catch (error) {
    console.error('Error loading data from local storage:', error)
  }
}

// 保存数据到本地存储
function saveToLocal() {
  try {
    const data = {
      nodes: [...nodes]
    }
    localStorage.setItem(STORAGE_KEY, JSON.stringify(data))
  } catch (error) {
    console.error('Error saving data to local storage:', error)
  }
}

// 添加节点函数
function addNode(x: number, y: number, parentId?: number) {
  const node = { id: nextId.value++, x, y, label: 'New Node', parentId }
  nodes.push(node)
  // 不要直接设置selectedNode.value，而是通过设置selectedId来更新选中状态
  selectedId.value = node.id
  saveToLocal()
  
  // 触发选中事件
  emit('nodeSelected', node)
}

// 移除节点函数
function removeNode(nodeId: number) {
  const index = nodes.findIndex(node => node.id === nodeId)
  if (index !== -1) {
    // 移除关联的节点
    const nodeToRemove = nodes[index]
    // 不要直接重新赋值nodes，而是使用splice方法
    const toRemove = new Set<number>()
    function dfs(cur: number) {
      toRemove.add(cur)
      nodes.filter(n => n.parentId === cur).forEach(ch => dfs(ch.id))
    }
    dfs(nodeId)
    for (let i = nodes.length - 1; i >= 0; i--) {
      if (toRemove.has(nodes[i].id)) {
        nodes.splice(i, 1)
      }
    }
    
    if (selectedId.value === nodeId) {
      selectedId.value = null
      emit('nodeSelected', null)
    }
    saveToLocal()
  }
}

// 更新节点函数
function updateNode(nodeId: number, data: Partial<Node>) {
  const node = nodes.find(node => node.id === nodeId)
  if (node) {
    Object.assign(node, data)
    saveToLocal()
  }
}

// 鼠标按下事件处理
function onMouseDown(e: MouseEvent) {
  // 阻止默认行为
  e.preventDefault()
  
  // 如果在编辑状态，忽略鼠标事件
  if (editingId.value !== null) return
  
  // 获取相对于容器的坐标
  const rect = containerRef.value?.getBoundingClientRect()
  if (!rect) return
  
  const x = (e.clientX - rect.left - pan.x) / scale.value
  const y = (e.clientY - rect.top - pan.y) / scale.value
  
  // 检查是否点击了节点
  const clickedNode = nodes.find(node => {
    const dx = node.x - x
    const dy = node.y - y
    // 简化的节点碰撞检测（假设节点是圆形，半径约为50像素）
    return dx * dx + dy * dy < 50 * 50
  })
  
  if (clickedNode) {
    // 点击了节点
    draggingId.value = clickedNode.id
    dragOffset.x = clickedNode.x - x
    dragOffset.y = clickedNode.y - y
    selectedId.value = clickedNode.id
    emit('nodeSelected', clickedNode)
  } else {
    // 点击了空白区域，取消选中
    selectedId.value = null
    emit('nodeSelected', null)
    
    // 检查是否在添加子节点状态
    if (isAdding.value && selectedParentId.value !== null) {
      // 添加子节点
      addNode(x, y, selectedParentId.value)
      selectedParentId.value = null
    } else if (spacePressed.value) {
      // 空格键按下时，开始平移视图
      isPanning.value = true
      panStart.x = e.clientX
      panStart.y = e.clientY
      panAtStart.x = pan.x
      panAtStart.y = pan.y
    }
  }
}

// 鼠标移动事件处理
function onMouseMove(e: MouseEvent) {
  // 阻止默认行为
  e.preventDefault()
  
  // 获取相对于容器的坐标
  const rect = containerRef.value?.getBoundingClientRect()
  if (!rect) return
  
  const x = (e.clientX - rect.left - pan.x) / scale.value
  const y = (e.clientY - rect.top - pan.y) / scale.value
  
  // 更新临时位置（用于添加子节点的预览）
  tempPos.x = x
  tempPos.y = y
  
  if (draggingId.value !== null) {
    // 拖拽节点
    const node = nodes.find(node => node.id === draggingId.value)
    if (node) {
      node.x = x + dragOffset.x
      node.y = y + dragOffset.y
      saveToLocal()
    }
  } else if (isPanning.value) {
    // 平移视图
    pan.x = panAtStart.x + (e.clientX - panStart.x)
    pan.y = panAtStart.y + (e.clientY - panStart.y)
  }
}

// 鼠标释放事件处理
function onMouseUp() {
  draggingId.value = null
  isPanning.value = false
}

// 键盘按下事件处理
function onKeydown(e: KeyboardEvent) {
  if (e.code === 'Space') {
    spacePressed.value = true
    // 防止空格键在输入框中插入空格
    e.preventDefault()
  }
  
  // 删除节点
  if (e.code === 'Delete' && selectedId.value !== null && editingId.value === null) {
    removeNode(selectedId.value)
    emit('nodeDelete', selectedId.value)
  }
}

// 键盘释放事件处理
function onKeyup(e: KeyboardEvent) {
  if (e.code === 'Space') {
    spacePressed.value = false
  }
}

// 处理滚轮事件（缩放）
function onWheel(e: WheelEvent) {
  e.preventDefault()
  
  // 获取滚轮增量
  const delta = e.deltaY < 0 ? 0.1 : -0.1
  
  // 计算新的缩放值
  const newScale = scale.value * (1 + delta)
  
  // 限制缩放范围
  if (newScale >= minScale && newScale <= maxScale) {
    // 获取鼠标位置
    const rect = containerRef.value?.getBoundingClientRect()
    if (!rect) return
    
    const mouseX = e.clientX - rect.left
    const mouseY = e.clientY - rect.top
    
    // 计算鼠标在世界坐标系中的位置
    const worldX = (mouseX - pan.x) / scale.value
    const worldY = (mouseY - pan.y) / scale.value
    
    // 更新缩放
    scale.value = newScale
    
    // 调整平移，使鼠标指向的世界坐标点保持不变
    pan.x = mouseX - worldX * scale.value
    pan.y = mouseY - worldY * scale.value
  }
}

// 开始添加子节点
function startAddChild(nodeId: number) {
  selectedParentId.value = nodeId
}

// 取消添加子节点
function cancelAddChild() {
  selectedParentId.value = null
}

// 获取连接路径函数
function getConnectionPath(node: Node) {
  if (!node.parentId) return ''
  
  const parent = nodes.find(n => n.id === node.parentId)
  if (!parent) return ''
  
  // 计算贝塞尔曲线控制点
  const dx = node.x - parent.x
  const dy = node.y - parent.y
  const distance = Math.sqrt(dx * dx + dy * dy)
  const offset = Math.min(distance * styleOpts.curvature, styleOpts.maxOffset)
  
  // 根据方向确定控制点位置
  const control1X = parent.x + (offset * dx) / distance
  const control1Y = parent.y + (offset * dy) / distance
  const control2X = node.x - (offset * dx) / distance
  const control2Y = node.y - (offset * dy) / distance
  
  return `M ${parent.x} ${parent.y} C ${control1X} ${control1Y}, ${control2X} ${control2Y}, ${node.x} ${node.y}`
}

// 显示右键菜单
function showContextMenu(nodeId: number, event: MouseEvent) {
  // 可以在这里实现右键菜单逻辑
  console.log('Context menu for node:', nodeId)
}

// 选中输入框文本
function selectAll() {
  nextTick(() => {
    editInput.value?.select()
  })
}

// 开始编辑节点
function startEdit(nodeId: number) {
  const node = nodes.find(node => node.id === nodeId)
  if (node) {
    editingId.value = nodeId
    editingText.value = node.label
    
    // 触发编辑事件
    emit('nodeEdit', nodeId)
  }
}

// 保存节点编辑
function saveEdit() {
  if (editingId.value !== null) {
    const node = nodes.find(node => node.id === editingId.value)
    if (node && editingText.value.trim() !== '') {
      node.label = editingText.value.trim()
      saveToLocal()
    }
    editingId.value = null
    editingText.value = ''
  }
}

// 取消节点编辑
function cancelEdit() {
  editingId.value = null
  editingText.value = ''
}

onBeforeUnmount(() => {
  window.removeEventListener('keydown', onKeydown)
  window.removeEventListener('keyup', onKeyup)
})
</script>

<template>
  <div 
    ref="containerRef"
    class="relative w-full h-full bg-gray-50"
    @mousedown="onMouseDown"
    @mousemove="onMouseMove"
    @mouseup="onMouseUp"
    @mouseleave="onMouseUp"
    @wheel="onWheel"
    @dblclick.stop
  >
    <!-- 内容容器 -->
    <div 
      class="absolute inset-0 transform-gpu"
      :style="{ transform: `translate(${pan.x}px, ${pan.y}px) scale(${scale})` }"
    >
      <!-- 连接线条 -->
      <svg class="absolute inset-0 w-full h-full pointer-events-none">
        <template v-for="node in nodes" :key="`line-${node.id}`">
          <g v-if="node.parentId">
            <path 
              :d="getConnectionPath(node)"
              :stroke="styleOpts.linkColor"
              :stroke-width="styleOpts.linkWidth"
              fill="none"
              class="transition-all duration-300 ease-in-out"
            />
          </g>
        </template>
      </svg>
      
      <!-- 子节点预览（虚线） -->
      <svg v-if="isAdding" class="absolute inset-0 w-full h-full pointer-events-none">
        <path 
          :d="getConnectionPath({ x: tempPos.x, y: tempPos.y, parentId: selectedParentId } as Node)"
          stroke="#9aa4b2"
          stroke-width="styleOpts.linkWidth"
          stroke-dasharray="5,5"
          fill="none"
        />
      </svg>
      
      <!-- 节点 -->
      <div 
        v-for="node in nodes" 
        :key="node.id"
        class="absolute transform -translate-x-1/2 -translate-y-1/2 transition-all duration-300 ease-in-out cursor-pointer"
        :style="{
          left: `${node.x}px`,
          top: `${node.y}px`,
          backgroundColor: styleOpts.nodeBg,
          borderColor: selectedId === node.id ? '#0366d6' : styleOpts.nodeBorder,
          borderWidth: selectedId === node.id ? '2px' : '1px',
          borderStyle: 'solid',
          borderRadius: '8px',
          padding: '12px 16px',
          minWidth: '80px',
          maxWidth: '200px',
          boxShadow: selectedId === node.id ? '0 0 0 2px rgba(3, 102, 214, 0.3)' : '0 1px 3px rgba(0, 0, 0, 0.1)',
          color: styleOpts.nodeText,
          fontSize: '14px',
          fontWeight: '500',
          textAlign: 'center',
          wordBreak: 'break-word',
          zIndex: selectedId === node.id ? '10' : '1'
        }"
        @contextmenu.stop.prevent="showContextMenu(node.id, $event)"
      >
        <!-- 编辑模式 -->
        <input 
          v-if="editingId === node.id"
          ref="editInput"
          v-model="editingText"
          class="w-full outline-none border-none bg-transparent text-center"
          @blur="saveEdit"
          @keydown.enter="saveEdit"
          @keydown.esc="cancelEdit"
          @mousedown.stop
          @click.stop
          @focus="selectAll"
        />
        <!-- 显示模式 -->
        <div v-else @dblclick="startEdit(node.id)">{{ node.label }}</div>
      </div>
    </div>
  </div>
</template>

<style scoped>
/* 自定义滚动条 */
::-webkit-scrollbar {
  width: 8px;
  height: 8px;
}

::-webkit-scrollbar-track {
  background: #f1f1f1;
}

::-webkit-scrollbar-thumb {
  background: #888;
  border-radius: 4px;
}

::-webkit-scrollbar-thumb:hover {
  background: #555;
}

/* 禁用文本选择 */
.no-select {
  user-select: none;
  -webkit-user-select: none;
  -moz-user-select: none;
  -ms-user-select: none;
}
</style>