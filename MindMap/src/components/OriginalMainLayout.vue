<template>
  <div class="flex h-screen overflow-hidden">
    <!-- 左侧边栏 -->
    <LeftSidebar :msg="msg" />

    <!-- 主内容区域 -->
    <main class="flex-1 overflow-hidden bg-gradient-to-br from-gray-50 to-gray-100">
      <OriginalMindMapCanvas v-if="showMindMap" ref="mindMapRef" @nodeSelected="onNodeSelected" />
      <MarkdownEditor v-else ref="markdownEditorRef" />
    </main>

    <!-- 右侧边栏 -->
    <RightSidebar 
      :selected-node="selectedNode" 
      :mind-map-ref="mindMapRef" 
      :show-mind-map="showMindMap" 
      @toggle-view="toggleView"
    />
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import OriginalMindMapCanvas from './OriginalMindMapCanvas.vue'
import LeftSidebar from './LeftSidebar.vue'
import RightSidebar from './RightSidebar.vue'
import MarkdownEditor from './Markdown/MarkdownEditor.vue'

interface Node {
  id: number
  x: number
  y: number
  label: string
  parentId?: number
}

const mindMapRef = ref()
const markdownEditorRef = ref()
const msg = 'Hello'
const selectedNode = ref<Node | null>(null)
const showMindMap = ref(true) // 控制显示思维导图还是Markdown编辑器

function onNodeSelected(node: Node | null) {
  selectedNode.value = node
}

function toggleView() {
  showMindMap.value = !showMindMap.value
}
</script>