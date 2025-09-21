<template>
    <!-- 主页面容器 -->
    <div class="flex">
    <!-- 左侧边栏：包含导航菜单 -->
    <!-- Vue的动态class绑定：根据leftIsCollapsed的值添加collapsed类 -->
    <!-- Vue的事件监听：@mouseenter和@mouseleave是Vue的事件修饰符 -->
    <aside class="h-screen bg-[#acb0b3] transition-all duration-300 overflow-hidden" :class="leftIsCollapsed ? 'w-12' : 'w-50'" @mouseenter="leftIsCollapsed = false" @mouseleave="leftIsCollapsed = true">
        <h1 class="text-[1.5em] bg-[#57595b] text-white overflow-hidden m-0 p-0">{{ msg }}</h1>
        <!-- Vue的列表渲染：v-for遍历navItems数组，:key为每个元素提供唯一标识符 -->
        <ul class="m-0 p-0 list-none">
            <li v-for="item in navItems" :key="item.id" class="py-1 px-4 border-b border-[#eee] cursor-pointer h-[20px] overflow-hidden hover:bg-[#57595b] hover:text-white">{{ item.text }}</li>
        </ul>
    </aside>

    <!-- 主内容区域：显示思维导图 -->
    <!-- Vue的组件引用：通过ref="mindMapRef"创建组件引用 -->
    <!-- Vue的事件监听：@nodeSelected监听子组件触发的自定义事件 -->
    <main class="flex-1 h-screen bg-[#f0f0f0] flex justify-center items-center">
        <MindMapCanvas ref="mindMapRef" @nodeSelected="onNodeSelected" />
    </main>

    <!-- 右侧边栏：节点操作面板 -->
    <!-- Vue的条件渲染：v-if根据selectedNode是否存在显示不同内容 -->
    <aside class="h-screen bg-[#acb0b3] transition-all duration-300 overflow-hidden pl-2 ml-2 float-right shadow-inner shadow-[#29627e]/30 italic text-[#29627e]" :class="rightIsCollapsed ? 'w-12' : 'w-50'" @mouseenter="rightIsCollapsed = false" @mouseleave="rightIsCollapsed = true">
        <div v-if="selectedNode" class="p-4 h-full flex flex-col">
            <h1 class="text-[1.2em] mb-4 text-[#29627e] border-b-2 border-[#29627e] pb-2">节点操作</h1>
            <div class="bg-[#29627e]/10 p-3 rounded-md mb-5 border border-[#29627e]/20">
                <h3 class="mb-2 text-[1.1em] text-[#29627e] font-semibold">{{ selectedNode.label }}</h3>
                <p class="text-[0.9em] text-[#666]">ID: {{ selectedNode.id }}</p>
            </div>
            <div class="flex flex-col gap-2.5">
                <button class="action-btn py-2.5 px-4 border border-[#acb0b3] bg-white rounded-md cursor-pointer text-sm text-[#29627e] font-medium transition-all text-left" @click="editNode">编辑</button>
                <button class="action-btn py-2.5 px-4 border border-[#acb0b3] bg-white rounded-md cursor-pointer text-sm text-[#29627e] font-medium transition-all text-left" @click="addChildNode">添加子节点</button>
                <button class="action-btn danger py-2.5 px-4 border border-[#fecaca] bg-white rounded-md cursor-pointer text-sm text-[#dc2626] font-medium transition-all text-left" @click="deleteNode">删除</button>
            </div>
        </div>
        <div v-else class="p-4 text-center text-[#666]">
            <h1 class="text-[1.2em] mb-3 text-[#29627e]">右侧边栏</h1>
            <p class="text-[0.9em] leading-relaxed">点击节点查看操作选项</p>
        </div>
    </aside>

    
    </div>
</template>

<script>
    // Vue 2 选项式API：导入子组件
    import MindMapCanvas from './MindMapCanvas.vue'
    export default {
        // Vue的组件注册：注册子组件
        components: {
            MindMapCanvas
        },
        // Vue的data选项：定义组件的数据
        data() {
            return {
                msg: 'Hello',
                navItems: [
                    { id: 1, text: '仪表盘', icon: 'fas fa-tachometer-alt' },
                    { id: 2, text: '用户管理', icon: 'fas fa-users' },
                    { id: 3, text: '数据分析', icon: 'fas fa-chart-line' },
                    { id: 4, text: '消息中心', icon: 'fas fa-envelope' },
                    { id: 5, text: '文件管理', icon: 'fas fa-folder' },
                    { id: 6, text: '系统设置', icon: 'fas fa-cog' },
                    { id: 7, text: '帮助文档', icon: 'fas fa-question-circle' }
                ],
                leftIsCollapsed: false, // 左侧边栏折叠状态
                rightIsCollapsed: false, // 右侧边栏折叠状态
                selectedNode: null // 当前选中的节点
            }
        },
        // Vue的methods选项：定义组件的方法
        methods: {
            // 事件处理函数：处理子组件触发的节点选中事件
            onNodeSelected(node) {
                this.selectedNode = node
            },
            // 节点编辑函数：调用子组件的startEditNode方法
            // 使用Vue的$refs访问子组件实例
            editNode() {
                if (this.selectedNode) {
                    this.$refs.mindMapRef.startEditNode(this.selectedNode.id)
                }
            },
            // 添加子节点函数：调用子组件的startAddChildNode方法
            addChildNode() {
                if (this.selectedNode) {
                    this.$refs.mindMapRef.startAddChildNode(this.selectedNode.id)
                }
            },
            // 删除节点函数：调用子组件的deleteSelectedNode方法，包含确认对话框
            deleteNode() {
                if (this.selectedNode) {
                    if (confirm(`确定要删除节点 "${this.selectedNode.label}" 及其所有子节点吗？`)) {
                        this.$refs.mindMapRef.deleteSelectedNode(this.selectedNode.id)
                    }
                }
            }
        }
    }
</script>

<style scoped>
/* 保持一些无法用Tailwind直接替代的过渡效果 */
.action-btn:hover {
    background: #f0f8ff;
    border-color: #29627e;
    transform: translateY(-1px);
    box-shadow: 0 2px 4px rgba(41, 98, 126, 0.2);
}

.action-btn.danger:hover {
    background: #fef2f2;
    border-color: #fca5a5;
    box-shadow: 0 2px 4px rgba(220, 38, 38, 0.2);
}
</style>