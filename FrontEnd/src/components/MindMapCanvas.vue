<script setup lang="ts">
import { ref, reactive, computed, onMounted, onBeforeUnmount, nextTick, watch } from 'vue'
import qtCommunicationService, { type GraphData, type Node as QtNode } from '../services/qtCommunicationService'

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
  
  // 连接到Qt后端的事件
  qtCommunicationService.on('graphUpdated', handleGraphUpdate)
  qtCommunicationService.on('nodeAdded', handleNodeAdded)
  qtCommunicationService.on('nodeRemoved', handleNodeRemoved)
  qtCommunicationService.on('nodeUpdated', handleNodeUpdated)
  
  // 尝试从Qt后端获取数据
  loadFromQtBackend()
})

// 从Qt后端加载数据
async function loadFromQtBackend() {
  if (qtCommunicationService.getIsConnected()) {
    try {
      const graph = await qtCommunicationService.getFullGraph()
      if (graph && graph.nodes && graph.nodes.length > 0) {
        // 转换Qt节点格式到Vue组件使用的格式
        const vueNodes = graph.nodes.map((qtNode: QtNode, index: number) => ({
          id: parseInt(qtNode.id),
          x: qtNode.x || 400 + (index * 50),
          y: qtNode.y || 300 + (index % 2 === 0 ? -50 : 50),
          label: qtNode.text,
          parentId: qtNode.parent ? parseInt(qtNode.parent) : undefined
        }))
        
        // 清空现有节点并添加新节点
        nodes.length = 0
        vueNodes.forEach((node: Node) => nodes.push(node))
        
        // 更新nextId
        nextId.value = Math.max(...nodes.map(n => n.id), 1) + 1
      } else {
        // 如果Qt后端没有数据，使用本地数据或创建根节点
        loadFromLocal()
        if (nodes.length === 0) {
          const el = containerRef.value
          const rect = el?.getBoundingClientRect()
          const cx = rect ? rect.width / 2 : 400
          const cy = rect ? rect.height / 2 : 300
          const rootNode = { id: nextId.value++, x: cx, y: cy, label: 'Root' }
          nodes.push(rootNode)
          saveToLocal()
          
          // 将根节点同步到Qt后端
          qtCommunicationService.addNode(rootNode.id.toString(), rootNode.label)
        }
      }
    } catch (error) {
      console.error('Error loading data from Qt backend:', error)
      loadFromLocal()
    }
  } else {
    // 如果没有连接到Qt后端，使用本地数据
    loadFromLocal()
    if (nodes.length === 0) {
      const el = containerRef.value
      const rect = el?.getBoundingClientRect()
      const cx = rect ? rect.width / 2 : 400
      const cy = rect ? rect.height / 2 : 300
      nodes.push({ id: nextId.value++, x: cx, y: cy, label: 'Root' })
      saveToLocal()
    }
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
  
  // 同步到Qt后端
  qtCommunicationService.addNode(node.id.toString(), node.label, parentId?.toString())
  if (parentId) {
    // 创建父子连接
    qtCommunicationService.addConnection(parentId.toString(), node.id.toString())
  }
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
    
    // 同步到Qt后端
    qtCommunicationService.removeNode(nodeId.toString())
  }
}

// 更新节点函数
function updateNode(nodeId: number, data: Partial<Node>) {
  const node = nodes.find(node => node.id === nodeId)
  if (node) {
    Object.assign(node, data)
    saveToLocal()
    
    // 同步到Qt后端
    qtCommunicationService.updateNode(node.id.toString(), node.label)
    
    // 如果节点位置发生变化，同步位置信息
    if ('x' in data || 'y' in data) {
      qtCommunicationService.updateNodePosition(node.id.toString(), node.x, node.y)
    }
    
    // 如果父节点发生变化，更新连接
    if ('parentId' in data) {
      // 先移除旧连接
      const oldConnections = nodes.filter(n => n.parentId === node.id)
      oldConnections.forEach(n => {
        if (node.parentId) {
          qtCommunicationService.removeConnection(node.parentId.toString(), node.id.toString())
        }
      })
      
      // 添加新连接
      if (node.parentId) {
        qtCommunicationService.addConnection(node.parentId.toString(), node.id.toString())
      }
    }
  }
}

// 处理Qt后端的图更新事件
function handleGraphUpdate(graph: GraphData) {
  console.log('Handling graph update from Qt:', graph)
  if (graph && graph.nodes && graph.nodes.length > 0) {
    // 转换Qt节点格式到Vue组件使用的格式
    const vueNodes = graph.nodes.map((qtNode: QtNode) => ({
      id: parseInt(qtNode.id),
      x: qtNode.x || 400,
      y: qtNode.y || 300,
      label: qtNode.text,
      parentId: qtNode.parent ? parseInt(qtNode.parent) : undefined
    }))
    
    // 清空现有节点并添加新节点
    nodes.length = 0
    vueNodes.forEach((node: Node) => nodes.push(node))
    
    // 更新nextId
    nextId.value = Math.max(...nodes.map(n => n.id), 1) + 1
  }
}

// 处理Qt后端的节点添加事件
function handleNodeAdded(node: QtNode) {
  console.log('Handling node added from Qt:', node)
  // 检查节点是否已存在
  const existingNode = nodes.find(n => n.id === parseInt(node.id))
  if (!existingNode) {
    nodes.push({
      id: parseInt(node.id),
      x: node.x || 400,
      y: node.y || 300,
      label: node.text,
      parentId: node.parent ? parseInt(node.parent) : undefined
    })
  }
}

// 处理Qt后端的节点删除事件
function handleNodeRemoved(id: string) {
  console.log('Handling node removed from Qt:', id)
  const nodeId = parseInt(id)
  const index = nodes.findIndex(n => n.id === nodeId)
  if (index !== -1) {
    nodes.splice(index, 1)
  }
}

// 处理Qt后端的节点更新事件
function handleNodeUpdated(node: QtNode) {
  console.log('Handling node updated from Qt:', node)
  const nodeId = parseInt(node.id)
  const existingNode = nodes.find(n => n.id === nodeId)
  if (existingNode) {
    existingNode.label = node.text
    if (node.x !== undefined) existingNode.x = node.x
    if (node.y !== undefined) existingNode.y = node.y
    if (node.parent) existingNode.parentId = parseInt(node.parent)
  }
}

onBeforeUnmount(() => {
  window.removeEventListener('keydown', onKeydown)
  window.removeEventListener('keyup', onKeyup)
  
  // 清理Qt后端事件监听器
  qtCommunicationService.off('graphUpdated', handleGraphUpdate)
  qtCommunicationService.off('nodeAdded', handleNodeAdded)
  qtCommunicationService.off('nodeRemoved', handleNodeRemoved)
  qtCommunicationService.off('nodeUpdated', handleNodeUpdated)
})

function onKeydown(e: KeyboardEvent) {
  if (e.code === 'Space') {
    spacePressed.value = true
    e.preventDefault()
  }
  if (e.key === 'Escape') {
    if (editingId.value !== null) {
      cancelEdit()
      return
    }
    if (isAdding.value) {
      selectedParentId.value = null
    }
  }
  if (e.key === 'Delete' || e.key === 'Backspace') {
    if (editingId.value === null && selectedId.value !== null) {
      e.preventDefault()
      deleteNodeAndDescendants(selectedId.value)
      selectedId.value = null
      saveToLocal()
    }
  }
}

function onKeyup(e: KeyboardEvent) {
  if (e.code === 'Space') {
    spacePressed.value = false
  }
}

function selectNode(nodeId: number) {
  if (editingId.value !== null) return
  selectedId.value = nodeId
  selectedParentId.value = null
  emit('nodeSelected', selectedNode.value)
}

function startAddChild(parentId: number) {
  if (editingId.value !== null) return
  selectedId.value = parentId
  selectedParentId.value = parentId
}

function toLocalPos(e: MouseEvent) {
  const rect = containerRef.value!.getBoundingClientRect()
  return { x: e.clientX - rect.left, y: e.clientY - rect.top }
}

// 获取根节点
const rootNode = computed(() => {
  return nodes.find(n => n.parentId === undefined) || null
})

// 屏幕坐标 -> 世界坐标（反变换）
function toWorldPos(local: { x: number, y: number }) {
  return { x: (local.x - pan.x) / scale.value, y: (local.y - pan.y) / scale.value }
}

// 计算节点相对于根节点的绝对位置
function getNodeAbsolutePos(node: Node): {x: number, y: number} {
  // 由于所有节点的位置已经存储为相对于根节点的位置，直接返回即可
  return { x: node.x, y: node.y }
}

// 将世界坐标转换为相对于根节点的坐标
function worldToRelativeToRoot(worldPos: {x: number, y: number}): {x: number, y: number} {
  // 由于我们的节点已经存储为相对于根节点的坐标，这里需要将世界坐标转换为相对根节点的坐标
  const relativeWorldPos = toWorldPos(toLocalPos({ clientX: worldPos.x, clientY: worldPos.y } as MouseEvent))
  if (!rootNode.value) {
    return relativeWorldPos
  }
  return {
    x: relativeWorldPos.x - rootNode.value.x,
    y: relativeWorldPos.y - rootNode.value.y
  }
}

// 将相对于根节点的坐标转换为世界坐标
function relativeToRootToWorld(relativePos: {x: number, y: number}): {x: number, y: number} {
  // 将相对于根节点的坐标转换为世界坐标
  if (!rootNode.value) {
    return relativePos
  }
  const worldPos = {
    x: relativePos.x + rootNode.value.x,
    y: relativePos.y + rootNode.value.y
  }
  return worldPos
}

function onMouseMove(e: MouseEvent) {
  const local = toLocalPos(e)

  // 画布平移
  if (isPanning.value) {
    pan.x = panAtStart.x + (local.x - panStart.x)
    pan.y = panAtStart.y + (local.y - panStart.y)
    return
  }

  const world = toWorldPos(local)

  // 节点拖拽
  if (draggingId.value !== null) {
    const n = nodes.find(x => x.id === draggingId.value)!
    const newX = world.x - dragOffset.x
    const newY = world.y - dragOffset.y
    
    if (!rootNode.value || n.id === rootNode.value.id) {
      // 拖拽根节点时，所有节点都一起移动
      const dx = newX - n.x
      const dy = newY - n.y
      n.x = newX
      n.y = newY
      // 更新所有子节点的位置
      nodes.forEach(node => {
        if (node.id !== n.id) {
          node.x += dx
          node.y += dy
        }
      })
    } else {
      // 拖拽非根节点时，只移动该节点
      n.x = newX
      n.y = newY
    }
    return
  }

  // 添加子节点预览
  if (isAdding.value) {
    tempPos.x = world.x
    tempPos.y = world.y
  }
}

function onDblClickCanvas() {
  if (!isAdding.value) return
  const parentId = selectedParentId.value!
  // 使用我们的addNode函数添加节点，确保与Qt后端同步
  addNode(tempPos.x, tempPos.y, parentId)
  selectedId.value = null
  selectedParentId.value = null
}

function onMouseDownNode(e: MouseEvent, n: Node) {
  if (editingId.value !== null) return
  selectedId.value = n.id
  const world = toWorldPos(toLocalPos(e))
  draggingId.value = n.id
  dragOffset.x = world.x - n.x
  dragOffset.y = world.y - n.y
}

function onMouseUp() {
  draggingId.value = null
  isPanning.value = false
}

function onDblClickNode(n: Node) {
  editingId.value = n.id
  editingText.value = n.label
  selectedParentId.value = null
  selectedId.value = n.id
  nextTick(() => {
    const el = document.getElementById(`edit-${n.id}`) as HTMLInputElement | null
    el?.focus()
    el?.select()
  })
}

function confirmEdit() {
  if (editingId.value === null) return
  const nodeId = editingId.value
  const newLabel = editingText.value.trim()
  if (newLabel) {
    // 使用我们的updateNode函数更新节点，确保与Qt后端同步
    updateNode(nodeId, { label: newLabel })
  }
  editingId.value = null
}

function cancelEdit() {
  editingId.value = null
}

// 暴露给父组件的方法
function startEditNode(nodeId: number) {
  editingId.value = nodeId
  editingText.value = nodes.find(n => n.id === nodeId)?.label || ''
  nextTick(() => {
    const el = document.getElementById(`edit-${nodeId}`) as HTMLInputElement | null
    el?.focus()
    el?.select()
  })
}

function startAddChildNode(nodeId: number) {
  startAddChild(nodeId)
}

function deleteSelectedNode(nodeId: number) {
  // 删除节点及其所有子节点
  deleteNodeAndDescendants(nodeId)
  selectedId.value = null
  emit('nodeSelected', null)
}

// 暴露方法给父组件
defineExpose({
  startEditNode,
  startAddChildNode,
  deleteSelectedNode
})

const edges = computed(() =>
  nodes
    .filter(n => n.parentId != null)
    .map(n => {
      const p = nodes.find(x => x.id === n.parentId)!
      return { id: `e-${p.id}-${n.id}`, x1: p.x, y1: p.y, x2: n.x, y2: n.y }
    })
)

const selectedParent = computed(() =>
  nodes.find(n => n.id === selectedParentId.value) || null
)

const selectedNode = computed(() =>
  nodes.find(n => n.id === selectedId.value) || null
)

// 曲线路径（可调弧度）
function curvedPathD(x1: number, y1: number, x2: number, y2: number) {
  const mx = (x1 + x2) / 2
  const my = (y1 + y2) / 2
  const dx = x2 - x1
  const dy = y2 - y1
  const len = Math.hypot(dx, dy) || 1
  const nx = -dy / len
  const ny = dx / len
  const offset = Math.min(styleOpts.maxOffset, len * styleOpts.curvature)
  const cx = mx + nx * offset
  const cy = my + ny * offset
  return `M ${x1} ${y1} Q ${cx} ${cy} ${x2} ${y2}`
}

// 删除节点及其子树
function deleteNodeAndDescendants(id: number) {
  const toDelete = new Set<number>()
  function dfs(cur: number) {
    toDelete.add(cur)
    nodes.filter(n => n.parentId === cur).forEach(ch => dfs(ch.id))
  }
  dfs(id)
  for (let i = nodes.length - 1; i >= 0; i--) {
    if (toDelete.has(nodes[i].id)) {
      // 从Qt后端移除节点
      qtCommunicationService.removeNode(nodes[i].id.toString())
      nodes.splice(i, 1)
    }
  }
  saveToLocal()
}

// 中键或空格+左键 平移
function onMouseDownCanvas(e: MouseEvent) {
  if (e.button === 1 || (e.button === 0 && spacePressed.value)) {
    const local = toLocalPos(e)
    isPanning.value = true
    panStart.x = local.x
    panStart.y = local.y
    panAtStart.x = pan.x
    panAtStart.y = pan.y
    e.preventDefault()
  }
}

// 滚轮缩放（以鼠标为中心）
function onWheel(e: WheelEvent) {
  e.preventDefault()
  const delta = -e.deltaY
  const factor = Math.exp(delta * 0.001) // 平滑缩放
  const newScale = Math.min(maxScale, Math.max(minScale, scale.value * factor))
  const local = toLocalPos(e as any)
  const before = toWorldPos(local)
  scale.value = newScale
  const after = toWorldPos(local)
  // 调整 pan 以保持鼠标处世界坐标不变
  pan.x += (after.x - before.x) * scale.value
  pan.y += (after.y - before.y) * scale.value
}

// 重置视图
function resetView() {
  pan.x = 0
  pan.y = 0
  scale.value = 1
}

// 自动布局：将每个父节点的直接子节点环形分布
function autoLayout() {
  const childrenMap = new Map<number, Node[]>()
  nodes.forEach(n => {
    if (n.parentId != null) {
      if (!childrenMap.has(n.parentId)) childrenMap.set(n.parentId, [])
      childrenMap.get(n.parentId)!.push(n)
    }
  })
  childrenMap.forEach((children, pid) => {
    const p = nodes.find(n => n.id === pid)
    if (!p) return
    const count = children.length
    if (count === 0) return
    const radius = Math.max(120, 40 * count)
    children.forEach((c, i) => {
      const angle = (i / count) * Math.PI * 2
      // 自动布局时，子节点位置是相对于父节点计算的，由于父节点已经是相对于根节点的坐标
      // 所以这里计算出的子节点位置也自动相对于根节点
      c.x = p.x + radius * Math.cos(angle)
      c.y = p.y + radius * Math.sin(angle)
      
      // 同步到Qt后端
      qtCommunicationService.updateNodePosition(c.id.toString(), c.x, c.y)
    })
  })
  saveToLocal()
}

// 本地存储
function saveToLocal() {
  const payload = {
    nodes: nodes.map(n => ({ ...n })),
    nextId: nextId.value,
    pan: { ...pan },
    scale: scale.value,
    style: { ...styleOpts }
  }
  localStorage.setItem(STORAGE_KEY, JSON.stringify(payload))
  
  // 如果连接到Qt后端，将完整图数据同步到Qt后端
  if (qtCommunicationService.getIsConnected()) {
    try {
      qtCommunicationService.updateFullGraph({
        nodes: nodes.map(n => ({
          id: n.id.toString(),
          text: n.label,
          x: n.x,
          y: n.y,
          parent: n.parentId?.toString()
        })),
        links: nodes
          .filter(n => n.parentId)
          .map(n => ({
            source: n.parentId!.toString(),
            target: n.id.toString()
          }))
      })
    } catch (error) {
      console.error('Error syncing full graph to Qt backend:', error)
    }
  }
}

function loadFromLocal() {
  const raw = localStorage.getItem(STORAGE_KEY)
  if (!raw) return
  try {
    const payload = JSON.parse(raw)
    if (Array.isArray(payload.nodes)) {
      nodes.splice(0, nodes.length, ...payload.nodes)
    }
    if (typeof payload.nextId === 'number') nextId.value = payload.nextId
    if (payload.pan) { pan.x = payload.pan.x || 0; pan.y = payload.pan.y || 0 }
    if (typeof payload.scale === 'number') scale.value = payload.scale
    if (payload.style) Object.assign(styleOpts, payload.style)
  } catch {}
}

// 监听变化自动保存
watch(nodes, saveToLocal, { deep: true })
watch([() => pan.x, () => pan.y, scale], saveToLocal)
watch(styleOpts, saveToLocal, { deep: true })

const contentStyle = computed(() => ({
  transform: `translate(${pan.x}px, ${pan.y}px) scale(${scale.value})`
}))
</script>

<template>
  <div
    class="relative w-full h-full user-select-none overflow-hidden bg-[#f8f9fb]"
    ref="containerRef"
    @mousemove="onMouseMove"
    @mouseup="onMouseUp"
    @mouseleave="onMouseUp"
    @mousedown="onMouseDownCanvas"
    @dblclick="onDblClickCanvas"
    @wheel.passive.prevent="onWheel"
  >
    <!-- 工具栏 -->
    <div class="absolute top-2 left-2 z-10 bg-white/90 border border-gray-200 rounded-lg px-2 py-1.5 flex items-center gap-2 shadow-md">
      <button class="px-2 py-1 border border-gray-300 bg-white rounded text-xs cursor-pointer" @click="resetView">重置视图</button>
      <button class="px-2 py-1 border border-gray-300 bg-white rounded text-xs cursor-pointer" @click="autoLayout">自动布局</button>
      <span class="w-[1px] h-5 bg-gray-200 mx-[1px]"></span>
      <label class="text-xs text-gray-500 flex items-center gap-1">节点背景 <input type="color" v-model="styleOpts.nodeBg" /></label>
      <label class="text-xs text-gray-500 flex items-center gap-1">节点边框 <input type="color" v-model="styleOpts.nodeBorder" /></label>
      <label class="text-xs text-gray-500 flex items-center gap-1">文字 <input type="color" v-model="styleOpts.nodeText" /></label>
      <label class="text-xs text-gray-500 flex items-center gap-1">连线 <input type="color" v-model="styleOpts.linkColor" /></label>
      <label class="text-xs text-gray-500 flex items-center gap-1">线宽
        <input type="range" min="1" max="6" step="1" v-model.number="styleOpts.linkWidth" />
      </label>
      <label class="text-xs text-gray-500 flex items-center gap-1">弧度
        <input type="range" min="0" max="0.6" step="0.02" v-model.number="styleOpts.curvature" />
      </label>
    </div>

    <!-- 可缩放/平移内容 -->
    <div class="absolute inset-0 origin-top-left" :style="contentStyle">
      <svg class="absolute inset-0 w-full h-full pointer-events-none z-0">
        <g>
          <path
            v-for="e in edges"
            :key="e.id"
            :d="curvedPathD(e.x1, e.y1, e.x2, e.y2)"
            fill="none"
            :stroke="styleOpts.linkColor"
            :stroke-width="styleOpts.linkWidth"
          />
          <path
            v-if="isAdding && selectedParent"
            :d="curvedPathD(selectedParent.x, selectedParent.y, tempPos.x, tempPos.y)"
            fill="none"
            stroke="#4a90e2"
            stroke-dasharray="6,6"
            :stroke-width="styleOpts.linkWidth"
          />
        </g>
      </svg>

      <!-- 节点 - 所有节点位置都是相对于根节点计算的绝对位置 -->
      <div
        v-for="n in nodes"
        :key="n.id"
        class="node absolute -translate-x-1/2 -translate-y-1/2 cursor-grab z-1"
        :class="{ selected: selectedId === n.id }"
        :style="{ left: n.x + 'px', top: n.y + 'px' }"
        @mousedown.stop="onMouseDownNode($event, n)"
        @dblclick.stop="onDblClickNode(n)"
        @click.stop="selectNode(n.id)"
      >
        <div
          class="node-body px-2.5 py-1.5 rounded-lg text-xs whitespace-nowrap inline-flex items-center gap-1.5"
          :class="{ editing: editingId === n.id }"
          :style="{ background: styleOpts.nodeBg, borderColor: styleOpts.nodeBorder, color: styleOpts.nodeText }"
        >
          <template v-if="editingId === n.id">
            <input
              :id="`edit-${n.id}`"
              class="edit-input text-xs border-none outline-none p-0 m-0 bg-transparent w-[160px]"
              v-model="editingText"
              @keydown.enter.prevent="confirmEdit"
              @keydown.esc.stop.prevent="cancelEdit"
              @blur="confirmEdit"
            />
          </template>
          <template v-else>
            {{ n.label }}
          </template>
        </div>
        <div class="hint mt-1 text-[11px] text-gray-500 text-center" v-if="editingId !== n.id && !isAdding">单击选中，双击编辑，拖拽移动</div>
        <div class="hint adding mt-1 text-[11px] text-blue-500 text-center" v-else-if="selectedParentId === n.id">选择位置后双击画布放置</div>
      </div>

      <!-- 预览子节点 -->
      <div
        v-if="isAdding"
        class="node ghost absolute -translate-x-1/2 -translate-y-1/2"
        :style="{ left: tempPos.x + 'px', top: tempPos.y + 'px' }"
      >
        <div class="node-body px-2.5 py-1.5 rounded-lg text-xs whitespace-nowrap inline-flex items-center gap-1.5" :style="{ background: styleOpts.nodeBg, borderColor: styleOpts.nodeBorder, color: styleOpts.nodeText }">新节点</div>
        <div class="hint mt-1 text-[11px] text-gray-500 text-center">双击画布确认，Esc 取消</div>
      </div>
    </div>
  </div>
</template>

<style scoped>
/* 保留无法用Tailwind直接替代的复杂交互样式 */
.node.selected .node-body {
  box-shadow: 0 0 0 2px rgba(74,144,226,0.25);
  border-color: #4a90e2;
}

.node:active {
  cursor: grabbing;
}

.node-body {
  background: #fff;
  border: 1px solid #dcdfe6;
  box-shadow: 0 1px 4px rgba(0,0,0,0.08);
}

.del-btn {
  border: none;
  background: transparent;
  color: #ef4444;
  font-weight: bold;
  cursor: pointer;
  opacity: 0.0;
  transition: opacity .15s;
}

.node:hover .del-btn {
  opacity: 1;
}

.node-body.editing {
  box-shadow: 0 0 0 2px rgba(74,144,226,0.25);
  border-color: #4a90e2;
}

.node.ghost .node-body {
  opacity: 0.5;
  border-style: dashed;
}
</style>