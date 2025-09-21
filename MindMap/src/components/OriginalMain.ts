import { createApp } from 'vue'
import './index.css'
import App from './OriginalApp.vue'

// 初始化Vue应用
const app = createApp(App)

// 挂载应用
app.mount('#app')