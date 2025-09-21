<template>
  <aside 
    class="transition-all duration-300 overflow-hidden bg-gray-400 h-screen"
    :class="isCollapsed ? 'w-12' : 'w-[50px]'"
    @mouseenter="isCollapsed = false" 
    @mouseleave="isCollapsed = true">
    <h1 class="bg-gray-600 text-white text-2xl m-0 p-0 overflow-hidden">{{ msg }}</h1>
    <ul class="m-0 p-0 list-none">
      <li v-for="item in navItems" :key="item.id" class="p-[6px] px-4 border-b border-gray-300 cursor-pointer h-[20px] overflow-hidden hover:bg-gray-600 hover:text-white transition-colors">
        {{ item.text }}
      </li>
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
/* 保持一些无法用Tailwind直接替代的过渡效果 */
</style>