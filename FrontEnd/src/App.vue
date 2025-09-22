<script setup lang="ts">
import { getCurrentInstance } from 'vue'
// 导入主布局组件
import MainLayout from './components/MainLayout.vue'

// 从全局属性中获取Qt通信服务
//const { $qt } = getCurrentInstance()!.appContext.config.globalProperties
</script>

<template>
  <div class="min-h-screen bg-gray-50">
    <!-- 测试元素，用于验证CSS是否被正确加载 -->
    <div id="css-test-element">
      测试元素 - CSS已加载
    </div>
    
    <MainLayout />
    
  </div>
</template>

<style>
/* 不使用scoped样式 */
/* 此组件不包含任何自定义CSS，所有样式都通过Tailwind提供 */
</style>
