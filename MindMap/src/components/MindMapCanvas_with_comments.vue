<script setup lang="ts">
// Vue 3 Composition API：导入所需的所有响应式和生命周期钩子
import { ref, reactive, computed, onMounted, onBeforeUnmount, nextTick, watch } from 'vue'

// Vue的事件定义：使用defineEmits定义组件可以触发的自定义事件
const emit = defineEmits<{
  nodeSelected: [node: Node | null]
  nodeEdit: [nodeId: number]
  nodeAddChild: [nodeId: number]
  nodeDelete: [nodeId: number]
}>()

// TypeScript类型定义：节点类型
type Node = {
  id: number
  x: number // 相对于根节点的坐标
  y: number
  label: string
  parentId?: number
}

// 常量定义：本地存储的键名
const STORAGE_KEY = 'mindmap_v1'

// Vue的响应式引用：使用ref创建基本类型的响应式数据
const containerRef = ref<HTMLElement | null>(null) // 容器DOM引用
const nodes = reactive<Node[]>([]) // 节点数组，使用reactive创建响应式数组
const nextId = ref(1) // 下一个节点的ID

// 节点添加相关的响应式数据
const selectedParentId = ref<number | null>(null) // 当前选中的父节点ID
const isAdding = computed(() => selectedParentId.value !== null) // 计算属性：是否处于添加子节点状态
const tempPos = reactive({ x: 0, y: 0 }) // 临时位置（世界坐标）

// 拖拽状态（节点）
const draggingId = ref<number | null>(null) // 当前正在拖拽的节点ID
const dragOffset = reactive({ x: 0, y: 0 }) // 拖拽偏移量

// 编辑状态
const editingId = ref<number | null>(null) // 当前正在编辑的节点ID
const editingText = ref('') // 编辑框中的文本内容

// 选中状态（用于 Delete 删除）
const selectedId = ref<number | null>(null) // 当前选中的节点ID

// 视图缩放/平移（屏幕 = translate(pan) + scale(scale) 应用于 content 容器）
const pan = reactive({ x: 0, y: 0 }) // 平移量
const scale = ref(1) // 缩放比例
const minScale = 0.3 // 最小缩放比例
const maxScale = 2.5 // 最大缩放比例

// 平移拖拽
const isPanning = ref(false) // 是否处于平移状态
const spacePressed = ref(false) // 空格键是否被按下
const panStart = reactive({ x: 0, y: 0 }) // 平移开始时的本地坐标
const panAtStart = reactive({ x: 0, y: 0 }) // 平移开始时的平移量

// 样式自定义
const styleOpts = reactive({
  nodeBg: '#ffffff', // 节点背景色
  nodeBorder: '#dcdfe6', // 节点边框色
  nodeText: '#333333', // 节点文字颜色
  linkColor: '#9aa4b2', // 连线颜色
  linkWidth: 2, // 连线宽度
  curvature: 0.30,   // 与距离成比例的曲率系数
  maxOffset: 80      // 弧度上限
})

// Vue的生命周期钩子：组件挂载后的初始化操作
onMounted(() => {
  // 添加全局键盘事件监听
  window.addEventListener('keydown', onKeydown)
  window.addEventListener('keyup', onKeyup)
  // 从本地存储加载数据
  loadFromLocal()
  // 若无数据则创建根节点
  if (nodes.length === 0) {
    const el = containerRef.value
    const rect = el?.getBoundingClientRect()
    const cx = rect ? rect.width / 2 : 400
    const cy = rect ? rect.height / 2 : 300
    nodes.push({ id: nextId.value++, x: cx, y: cy, label: 'Root' })
    saveToLocal()
  }
  
  // 在DOM更新后，将第二个元素的位置设置为第一个元素的上半部分中心
  // Vue的nextTick：确保在DOM更新后执行回调
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

// Vue的生命周期钩子：组件卸载前的清理操作
onBeforeUnmount(() => {
  // 移除全局事件监听
  window.removeEventListener('keydown', onKeydown)
  window.removeEventListener('keyup', onKeyup)
})

// 键盘按下事件处理函数
function onKeydown(e: KeyboardEvent) {
  if (e.code === 'Space') {
    spacePressed.value = true
    e.preventDefault() // 阻止默认行为
  }
  if (e.key === 'Escape') {
    // 按下ESC键取消编辑或添加状态
    if (editingId.value !== null) {
      cancelEdit()
      return
    }
    if (isAdding.value) {
      selectedParentId.value = null
    }
  }
  if (e.key === 'Delete' || e.key === 'Backspace') {
    // 按下Delete或Backspace键删除选中的节点
    if (editingId.value === null && selectedId.value !== null) {
      e.preventDefault()
      deleteNodeAndDescendants(selectedId.value)
      selectedId.value = null
      saveToLocal()
    }
  }
}

// 键盘释放事件处理函数
function onKeyup(e: KeyboardEvent) {
  if (e.code === 'Space') {
    spacePressed.value = false
  }
}

// 选择节点函数
function selectNode(nodeId: number) {
  if (editingId.value !== null) return // 如果正在编辑则不允许选择
  selectedId.value = nodeId
  selectedParentId.value = null
  emit('nodeSelected', selectedNode.value) // 触发节点选中事件
}

// 开始添加子节点函数
function startAddChild(parentId: number) {
  if (editingId.value !== null) return // 如果正在编辑则不允许添加
  selectedId.value = parentId
  selectedParentId.value = parentId
}

// 将鼠标坐标转换为容器本地坐标
function toLocalPos(e: MouseEvent) {
  const rect = containerRef.value!.getBoundingClientRect()
  return { x: e.clientX - rect.left, y: e.clientY - rect.top }
}

// Vue的计算属性：获取根节点
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

// 鼠标移动事件处理函数
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

// 双击画布事件处理函数
function onDblClickCanvas() {
  if (!isAdding.value) return
  const parentId = selectedParentId.value!
  // 新节点的位置已经是相对于根节点的坐标
  nodes.push({
    id: nextId.value++,
    x: tempPos.x,
    y: tempPos.y,
    label: `Node ${nextId.value - 1}`,
    parentId
  })
  selectedId.value = null
  selectedParentId.value = null
  saveToLocal()
}

// 鼠标按下节点事件处理函数
function onMouseDownNode(e: MouseEvent, n: Node) {
  if (editingId.value !== null) return
  selectedId.value = n.id
  const world = toWorldPos(toLocalPos(e))
  draggingId.value = n.id
  dragOffset.x = world.x - n.x
  dragOffset.y = world.y - n.y
}

// 鼠标释放事件处理函数
function onMouseUp() {
  draggingId.value = null
  isPanning.value = false
}

// 双击节点事件处理函数
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

// 确认编辑函数
function confirmEdit() {
  if (editingId.value === null) return
  const n = nodes.find(x => x.id === editingId.value)
  if (n) n.label = editingText.value.trim() || n.label
  editingId.value = null
  saveToLocal()
}

// 取消编辑函数
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
  deleteNodeAndDescendants(nodeId)
  selectedId.value = null
  saveToLocal()
  emit('nodeSelected', null)
}

// Vue的defineExpose：将方法暴露给父组件
defineExpose({
  startEditNode,
  startAddChildNode,
  deleteSelectedNode
})

// Vue的计算属性：生成连线数据
const edges = computed(() =>
  nodes
    .filter(n => n.parentId != null)
    .map(n => {
      const p = nodes.find(x => x.id === n.parentId)!
      return { id: `e-${p.id}-${n.id}`, x1: p.x, y1: p.y, x2: n.x, y2: n.y }
    })
)

// Vue的计算属性：获取选中的父节点
const selectedParent = computed(() =>
  nodes.find(n => n.id === selectedParentId.value) || null
)

// Vue的计算属性：获取选中的节点
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
  // 深度优先搜索，找出所有需要删除的节点
  function dfs(cur: number) {
    toDelete.add(cur)
    nodes.filter(n => n.parentId === cur).forEach(ch => dfs(ch.id))
  }
  dfs(id)
  // 从后往前删除，避免索引问题
  for (let i = nodes.length - 1; i >= 0; i--) {
    if (toDelete.has(nodes[i].id)) nodes.splice(i, 1)
  }
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
  // 构建父节点到子节点的映射
  nodes.forEach(n => {
    if (n.parentId != null) {
      if (!childrenMap.has(n.parentId)) childrenMap.set(n.parentId, [])
      childrenMap.get(n.parentId)!.push(n)
    }
  })
  // 对每个父节点的子节点进行环形布局
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
    })
  })
  saveToLocal()
}

// 本地存储：保存数据到localStorage
function saveToLocal() {
  const payload = {
    nodes: nodes.map(n => ({ ...n })), // 深拷贝nodes数组
    nextId: nextId.value,
    pan: { ...pan },
    scale: scale.value,
    style: { ...styleOpts }
  }
  localStorage.setItem(STORAGE_KEY, JSON.stringify(payload))
}

// 本地存储：从localStorage加载数据
function loadFromLocal() {
  const raw = localStorage.getItem(STORAGE_KEY)
  if (!raw) return
  try {
    const payload = JSON.parse(raw)
    if (Array.isArray(payload.nodes)) {
      nodes.splice(0, nodes.length, ...payload.nodes) // 替换整个nodes数组
    }
    if (typeof payload.nextId === 'number') nextId.value = payload.nextId
    if (payload.pan) { pan.x = payload.pan.x || 0; pan.y = payload.pan.y || 0 }
    if (typeof payload.scale === 'number') scale.value = payload.scale
    if (payload.style) Object.assign(styleOpts, payload.style)
  } catch {}
}

// Vue的watch：监听数据变化自动保存
// deep: true表示深度监听数组内部元素的变化
watch(nodes, saveToLocal, { deep: true })
// 监听视图状态变化
watch([() => pan.x, () => pan.y, scale], saveToLocal)
// 监听样式变化
watch(styleOpts, saveToLocal, { deep: true })

// Vue的计算属性：动态生成内容容器的样式
const contentStyle = computed(() => ({
  transform: `translate(${pan.x}px, ${pan.y}px) scale(${scale.value})`
}))
</script>

<template>
  <!-- 思维导图容器 -->
  <!-- Vue的事件修饰符：@wheel.passive.prevent用于优化滚轮事件性能 -->
  <div
    class="map-container"
    ref="containerRef" <!-- Vue的ref属性：获取DOM引用 -->
    @mousemove="onMouseMove"
    @mouseup="onMouseUp"
    @mouseleave="onMouseUp"
    @mousedown="onMouseDownCanvas"
    @dblclick="onDblClickCanvas"
    @wheel.passive.prevent="onWheel"
  >
    <!-- 工具栏 -->
    <div class="toolbar">
      <button class="btn" @click="resetView">重置视图</button>
      <button class="btn" @click="autoLayout">自动布局</button>
      <span class="sep"></span>
      <!-- Vue的双向数据绑定：v-model将input的值绑定到styleOpts对象的属性 -->
      <label class="lbl">节点背景 <input type="color" v-model="styleOpts.nodeBg" /></label>
      <label class="lbl">节点边框 <input type="color" v-model="styleOpts.nodeBorder" /></label>
      <label class="lbl">文字 <input type="color" v-model="styleOpts.nodeText" /></label>
      <label class="lbl">连线 <input type="color" v-model="styleOpts.linkColor" /></label>
      <label class="lbl">线宽
        <input type="range" min="1" max="6" step="1" v-model.number="styleOpts.linkWidth" />
      </label>
      <label class="lbl">弧度
        <input type="range" min="0" max="0.6" step="0.02" v-model.number="styleOpts.curvature" />
      </label>
    </div>

    <!-- 可缩放/平移内容 -->
    <!-- Vue的动态样式：:style绑定计算属性contentStyle -->
    <div class="content" :style="contentStyle">
      <!-- 连线层 -->
      <svg class="edges">
        <g>
          <!-- 渲染已有的连线 -->
          <path
            v-for="e in edges" 
            :key="e.id"
            :d="curvedPathD(e.x1, e.y1, e.x2, e.y2)"
            fill="none"
            :stroke="styleOpts.linkColor"
            :stroke-width="styleOpts.linkWidth"
          />
          <!-- 渲染添加子节点时的预览连线 -->
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

      <!-- 节点层 - 所有节点位置都是相对于根节点计算的绝对位置 -->
      <div
        v-for="n in nodes" <!-- Vue的列表渲染 -->
        :key="n.id"
        class="node"
        :class="{ selected: selectedId === n.id }" <!-- Vue的动态class -->
        :style="{ left: n.x + 'px', top: n.y + 'px' }" <!-- Vue的动态style -->
        @mousedown.stop="onMouseDownNode($event, n)" <!-- Vue的事件修饰符：stop阻止事件冒泡 -->
        @dblclick.stop="onDblClickNode(n)"
        @click.stop="selectNode(n.id)"
      >
        <div
          class="node-body"
          :class="{ editing: editingId === n.id }"
          :style="{ background: styleOpts.nodeBg, borderColor: styleOpts.nodeBorder, color: styleOpts.nodeText }"
        >
          <!-- Vue的条件渲染：使用template包裹不同状态的内容 -->
          <template v-if="editingId === n.id">
            <input
              :id="`edit-${n.id}`"
              class="edit-input"
              v-model="editingText" <!-- Vue的双向数据绑定 -->
              @keydown.enter.prevent="confirmEdit" <!-- Vue的按键修饰符 -->
              @keydown.esc.stop.prevent="cancelEdit"
              @blur="confirmEdit"
            />
          </template>
          <template v-else>
            {{ n.label }} <!-- Vue的文本插值 -->
          </template>
        </div>
        <!-- 提示文本 -->
        <div class="hint" v-if="editingId !== n.id && !isAdding">单击选中，双击编辑，拖拽移动</div>
        <div class="hint adding" v-else-if="selectedParentId === n.id">选择位置后双击画布放置</div>
      </div>

      <!-- 预览子节点 -->
      <div
        v-if="isAdding"
        class="node ghost"
        :style="{ left: tempPos.x + 'px', top: tempPos.y + 'px' }"
      >
        <div class="node-body" :style="{ background: styleOpts.nodeBg, borderColor: styleOpts.nodeBorder, color: styleOpts.nodeText }">新节点</div>
        <div class="hint">双击画布确认，Esc 取消</div>
      </div>
    </div>
  </div>
</template>

<style scoped>
/* scoped样式：使用data-v-xxx前缀确保只应用于当前组件 */
.map-container {
  position: relative;
  width: 100%;
  height: 100%;
  user-select: none;
  overflow: hidden;
  background: #f8f9fb;
}

.toolbar {
  position: absolute;
  top: 8px;
  left: 8px;
  z-index: 10;
  background: rgba(255,255,255,0.9);
  border: 1px solid #e5e7eb;
  border-radius: 8px;
  padding: 6px 8px;
  display: flex;
  align-items: center;
  gap: 8px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.06);
}

.btn {
  padding: 4px 8px;
  border: 1px solid #d1d5db;
  background: #ffffff;
  border-radius: 6px;
  cursor: pointer;
  font-size: 12px;
}

.sep {
  width: 1px;
  height: 20px;
  background: #e5e7eb;
  margin: 0 2px;
}

.lbl {
  font-size: 12px;
  color: #4b5563;
  display: flex;
  align-items: center;
  gap: 4px;
}

.content {
  position: absolute;
  inset: 0;
  transform-origin: 0 0;
}

.edges {
  position: absolute;
  inset: 0;
  width: 100%;
  height: 100%;
  pointer-events: none;
  z-index: 0;
}

.node {
  position: absolute;
  transform: translate(-50%, -50%); /* 使节点中心点对齐坐标 */
  cursor: grab;
  z-index: 1;
}

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
  padding: 6px 10px;
  border-radius: 8px;
  box-shadow: 0 1px 4px rgba(0,0,0,0.08);
  font-size: 12px;
  color: #333;
  white-space: nowrap;
  display: inline-flex;
  align-items: center;
  gap: 6px;
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

.edit-input {
  font-size: 12px;
  border: none;
  outline: none;
  padding: 0;
  margin: 0;
  background: transparent;
  width: 160px;
}

.node .hint {
  margin-top: 4px;
  font-size: 11px;
  color: #999;
  text-align: center;
}

.node .hint.adding {
  color: #4a90e2;
}

.node.ghost .node-body {
  opacity: 0.5;
  border-style: dashed;
}
</style>