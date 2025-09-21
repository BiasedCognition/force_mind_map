<script setup lang="ts">
// 导入主布局组件
import OriginalMainLayout from './OriginalMainLayout.vue'
</script>

<template>
  <div class="min-h-screen bg-gray-50">
    <!-- 主布局组件 -->
    <OriginalMainLayout />
    
    <!-- TailwindCSS 渲染验证元素 -->
    <div id="css-test-element" class="hidden">
      <p>这是一个非Tailwind测试规则，用于验证CSS文件是否被加载</p>
    </div>
  </div>
</template>

<style>
/* 不使用scoped样式 */
/* 此组件不包含任何自定义CSS，所有样式都通过Tailwind提供 */
</style>