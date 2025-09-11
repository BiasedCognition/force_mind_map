<template>
  <aside class="Sidebar" :class="{ collapsed: isCollapsed }" @mouseenter="isCollapsed = false" @mouseleave="isCollapsed = true">
    <h1 class="Sidebar-title">{{ msg }}</h1>
    <ul>
      <li v-for="item in navItems" :key="item.id" class="Sidebar-item">{{ item.text }}</li>
    </ul>
  </aside>
</template>

<script setup lang="ts">
import { ref, reactive } from 'vue'

interface NavItem {
  id: number
  text: string
  icon?: string
}

// 接收props
const props = defineProps<{
  msg?: string
}>()

// 导航菜单项数据
const navItems = reactive<NavItem[]>([
  { id: 1, text: '仪表盘', icon: 'fas fa-tachometer-alt' },
  { id: 2, text: '用户管理', icon: 'fas fa-users' },
  { id: 3, text: '数据分析', icon: 'fas fa-chart-line' },
  { id: 4, text: '消息中心', icon: 'fas fa-envelope' },
  { id: 5, text: '文件管理', icon: 'fas fa-folder' },
  { id: 6, text: '系统设置', icon: 'fas fa-cog' },
  { id: 7, text: '帮助文档', icon: 'fas fa-question-circle' }
])

// 侧边栏折叠状态
const isCollapsed = ref(false)
</script>

<style scoped>
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

ul {
  margin: 0;
  padding: 0;
  list-style-type: none;
}
</style>