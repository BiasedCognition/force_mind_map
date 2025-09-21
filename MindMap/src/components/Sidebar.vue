<template>
    <div class="flex">
    <aside class="transition-all duration-300 overflow-hidden bg-gray-400 h-screen" :class="leftIsCollapsed ? 'w-[50px]' : 'w-[200px]'" @mouseenter="leftIsCollapsed = false" @mouseleave="leftIsCollapsed = true">
        <h1 class="text-2xl m-0 p-0 bg-gray-700 text-white overflow-hidden">{{ msg }}</h1>
        <ul class="m-0 p-0 list-none">
            <li v-for="item in navItems" :key="item.id" class="py-1 px-4 border-b border-gray-200 cursor-pointer h-5 overflow-hidden hover:bg-gray-700 hover:text-white">{{ item.text }}</li>
        </ul>
    </aside>

    <main class="flex-1 h-screen bg-gray-100 flex justify-center items-center">
        <MindMapCanvas ref="mindMapRef" @nodeSelected="onNodeSelected" />
    </main>

    <aside class="transition-all duration-300 overflow-y-auto" :class="rightIsCollapsed ? 'w-[50px]' : 'w-[200px]'" @mouseenter="rightIsCollapsed = false" @mouseleave="rightIsCollapsed = true">
        <div v-if="selectedNode" class="p-4 h-full flex flex-col">
            <h1 class="text-lg mb-4 text-blue-700 border-b-2 border-blue-700 pb-2">节点操作</h1>
            <div class="bg-blue-50 p-3 rounded-lg mb-5 border border-blue-100">
                <h3 class="mb-2 text-blue-700 font-semibold text-base">节点：{{ selectedNode.label }}</h3>
                <p class="m-0 text-sm text-gray-600">ID: {{ selectedNode.id }}</p>
            </div>
            <div class="flex flex-col gap-2.5">
                <button class="action-btn px-4 py-2 border border-gray-400 bg-white rounded text-base text-blue-700 font-medium transition-all duration-200 text-left" @click="editNode">编辑</button>
                <button class="action-btn px-4 py-2 border border-gray-400 bg-white rounded text-base text-blue-700 font-medium transition-all duration-200 text-left" @click="addChildNode">添加子节点</button>
                <button class="action-btn danger px-4 py-2 border border-red-200 bg-white rounded text-base text-red-600 font-medium transition-all duration-200 text-left" @click="deleteNode">删除</button>
            </div>
        </div>
        <div v-else class="p-4 text-center text-gray-600">
            <h1 class="text-lg mb-3 text-blue-700">右侧边栏</h1>
            <p class="text-sm leading-relaxed">点击节点查看操作选项</p>
        </div>
    </aside>

    
    </div>
</template>

<script>
    import MindMapCanvas from './MindMapCanvas.vue'
    export default {
        components: {
            MindMapCanvas
        },
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
                leftIsCollapsed: false,
                rightIsCollapsed: false,
                selectedNode: null
            }
        },
        methods: {
            onNodeSelected(node) {
                this.selectedNode = node
            },
            editNode() {
                if (this.selectedNode) {
                    this.$refs.mindMapRef.startEditNode(this.selectedNode.id)
                }
            },
            addChildNode() {
                if (this.selectedNode) {
                    this.$refs.mindMapRef.startAddChildNode(this.selectedNode.id)
                }
            },
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
/* 保留无法用Tailwind直接替代的复杂交互样式 */
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