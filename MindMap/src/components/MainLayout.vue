<template>
  <div class="MainPage">
    <!-- 左侧边栏 -->
    <LeftSidebar :msg="msg" />

    <!-- 主内容区域 -->
    <main class="MindMap">
      <MindMapCanvas v-if="showMindMap" ref="mindMapRef" @nodeSelected="onNodeSelected" />
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
import MindMapCanvas from './MindMapCanvas.vue'
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

<style scoped>
* {
  margin: 0;
  padding: 0;
}

.MainPage {
  display: flex;
}

.MindMap {
  flex: 1;
  height: 100vh;
  background-color: #f0f0f0;
  display: flex;
  justify-content: center;
  align-items: center;
}
</style>