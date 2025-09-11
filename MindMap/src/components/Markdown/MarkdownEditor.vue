<template>
  <div class="markdown-editor-container">
    <div class="editor-toolbar">
      <h2>Markdown Editor</h2>
      <div class="size-controls">
        <label>
          输入框宽度: {{ inputWidth }}%
          <input type="range" min="20" max="80" v-model="inputWidth" />
        </label>
        <label>
          输入框高度: {{ inputHeight }}px
          <input type="range" min="100" max="800" v-model="inputHeight" />
        </label>
        <label>
          布局: 
          <select v-model="layoutType">
            <option value="horizontal">水平布局</option>
            <option value="vertical">垂直布局</option>
            <option value="split">分屏布局</option>
          </select>
        </label>
      </div>
    </div>
    
    <div class="editor-content" :class="layoutType">
      <div class="editor-section">
        <h3>输入区</h3>
        <textarea 
          ref="editorTextarea"
          v-model="markdownText" 
          @input="parseMarkdown"
          :style="{ width: `${inputWidth}%`, height: `${inputHeight}px` }"
          placeholder="在此输入Markdown文本...\n\n# 标题示例\n## 二级标题\n### 三级标题\n\n**粗体文本** 和 *斜体文本*\n\n- 列表项1\n- 列表项2\n\n> 引用文本\n\n`代码块`\n\n```javascript\nfunction example() {\n  console.log('代码块示例');\n}\n```\n\n[链接文本](https://example.com)\n\n![图片描述](https://via.placeholder.com/150)"
        ></textarea>
      </div>
      
      <div class="preview-section">
        <h3>预览区</h3>
        <div 
          class="markdown-preview"
          :style="{ width: layoutType === 'split' ? '100%' : `${100 - inputWidth}%` }"
          v-html="htmlContent"
        ></div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';

const markdownText = ref('');
const htmlContent = ref('');
const inputWidth = ref(50);
const inputHeight = ref(400);
const layoutType = ref('split');
const editorTextarea = ref<HTMLTextAreaElement | null>(null);

// 自定义Markdown解析器
function parseMarkdown() {
  let text = markdownText.value;
  let html = '';
  
  // 1. 解析代码块 (```)
  html = parseCodeBlocks(text);
  
  // 2. 解析内联代码 (`code`)
  html = parseInlineCode(html);
  
  // 3. 解析标题 (# 标题)
  html = parseHeadings(html);
  
  // 4. 解析粗体和斜体 (**粗体** 或 __粗体__, *斜体* 或 _斜体_)
  html = parseBoldAndItalic(html);
  
  // 5. 解析引用 (> 引用)
  html = parseBlockquotes(html);
  
  // 6. 解析无序列表 (- 列表项)
  html = parseUnorderedLists(html);
  
  // 7. 解析有序列表 (1. 列表项)
  html = parseOrderedLists(html);
  
  // 8. 解析链接 ([链接文本](url))
  html = parseLinks(html);
  
  // 9. 解析图片 (![图片描述](url))
  html = parseImages(html);
  
  // 10. 解析段落 (将换行符转换为<br>，连续空行转换为<p>)
  html = parseParagraphs(html);
  
  htmlContent.value = html;
}

// 解析代码块
function parseCodeBlocks(text: string): string {
  const codeBlockRegex = /```([\s\S]*?)```/g;
  return text.replace(codeBlockRegex, (match, code) => {
    // 提取语言标记
    const lines = code.split('\n');
    let language = '';
    let codeContent = '';
    
    if (lines.length > 0 && lines[0].trim()) {
      language = lines[0].trim();
      codeContent = lines.slice(1).join('\n');
    } else {
      codeContent = code;
    }
    
    return `<div class="highlight"><pre><code class="language-${language}">${escapeHtml(codeContent).replace(/\n/g, '<br>')}</code></pre></div>`;
  });
}

// 解析内联代码
function parseInlineCode(text: string): string {
  const inlineCodeRegex = /`([^`]+)`/g;
  return text.replace(inlineCodeRegex, '<code>$1</code>');
}

// 解析标题
function parseHeadings(text: string): string {
  let result = text;
  // 从h1到h6
  for (let i = 6; i >= 1; i--) {
    const headingRegex = new RegExp(`^${'#'.repeat(i)} (.*?)$`, 'gm');
    result = result.replace(headingRegex, `<h${i}>$1</h${i}>`);
  }
  return result;
}

// 解析粗体和斜体
function parseBoldAndItalic(text: string): string {
  // 先解析粗体
  let result = text.replace(/\*\*(.+?)\*\*/g, '<strong>$1</strong>');
  result = result.replace(/__(.+?)__/g, '<strong>$1</strong>');
  
  // 再解析斜体
  result = result.replace(/\*(.+?)\*/g, '<em>$1</em>');
  result = result.replace(/_(.+?)_/g, '<em>$1</em>');
  
  return result;
}

// 解析引用
function parseBlockquotes(text: string): string {
  const blockquoteRegex = /^> (.*?)$/gm;
  return text.replace(blockquoteRegex, '<blockquote><p>$1</p></blockquote>');
}

// 解析无序列表
function parseUnorderedLists(text: string): string {
  const ulRegex = /^- (.+?)$/gm;
  
  // 先找出所有列表项
  const matches = Array.from(text.matchAll(ulRegex));
  if (matches.length === 0) return text;
  
  // 找出列表的起始和结束行
  let result = text;
  let currentUl = '';
  let startIndex = -1;
  let lastIndex = -1;
  
  const lines = text.split('\n');
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    if (ulRegex.test(line)) {
      if (startIndex === -1) {
        startIndex = i;
      }
      lastIndex = i;
      currentUl += `<li>${line.replace(/^- /, '')}</li>`;
    } else if (startIndex !== -1) {
      // 列表结束
      lines[startIndex] = `<ul>${currentUl}</ul>`;
      for (let j = startIndex + 1; j <= lastIndex; j++) {
        lines[j] = '';
      }
      currentUl = '';
      startIndex = -1;
    }
  }
  
  // 处理最后一个列表
  if (startIndex !== -1) {
    lines[startIndex] = `<ul>${currentUl}</ul>`;
    for (let j = startIndex + 1; j <= lastIndex; j++) {
      lines[j] = '';
    }
  }
  
  return lines.filter(line => line !== '').join('\n');
}

// 解析有序列表
function parseOrderedLists(text: string): string {
  const olRegex = /^\d+\. (.+?)$/gm;
  
  // 先找出所有列表项
  const matches = Array.from(text.matchAll(olRegex));
  if (matches.length === 0) return text;
  
  // 找出列表的起始和结束行
  let result = text;
  let currentOl = '';
  let startIndex = -1;
  let lastIndex = -1;
  let expectedNumber = 1;
  
  const lines = text.split('\n');
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const match = olRegex.exec(line);
    olRegex.lastIndex = 0;
    
    if (match) {
      const number = parseInt(match[0].split('.')[0]);
      if (startIndex === -1) {
        startIndex = i;
        expectedNumber = number;
      }
      lastIndex = i;
      currentOl += `<li>${match[1]}</li>`;
    } else if (startIndex !== -1) {
      // 列表结束
      lines[startIndex] = `<ol start="${expectedNumber}">${currentOl}</ol>`;
      for (let j = startIndex + 1; j <= lastIndex; j++) {
        lines[j] = '';
      }
      currentOl = '';
      startIndex = -1;
    }
  }
  
  // 处理最后一个列表
  if (startIndex !== -1) {
    lines[startIndex] = `<ol start="${expectedNumber}">${currentOl}</ol>`;
    for (let j = startIndex + 1; j <= lastIndex; j++) {
      lines[j] = '';
    }
  }
  
  return lines.filter(line => line !== '').join('\n');
}

// 解析链接
function parseLinks(text: string): string {
  const linkRegex = /\[(.*?)\]\((.*?)\)/g;
  return text.replace(linkRegex, '<a href="$2" target="_blank" rel="noopener noreferrer">$1</a>');
}

// 解析图片
function parseImages(text: string): string {
  const imageRegex = /!\[(.*?)\]\((.*?)\)/g;
  return text.replace(imageRegex, '<img src="$2" alt="$1" />');
}

// 解析段落
function parseParagraphs(text: string): string {
  const lines = text.split('\n');
  let result = '';
  let currentParagraph = '';
  
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i].trim();
    
    // 如果是空行，且当前有段落内容，则闭合段落
    if (line === '' && currentParagraph !== '') {
      result += `<p>${currentParagraph}</p>`;
      currentParagraph = '';
    } 
    // 如果行不是空行，且不是HTML标签（如标题、列表等），则添加到当前段落
    else if (line !== '' && !line.startsWith('<') && !line.endsWith('>')) {
      if (currentParagraph !== '') currentParagraph += '\n';
      currentParagraph += line;
    } 
    // 否则，直接添加该行（可能是HTML标签）
    else if (line !== '') {
      if (currentParagraph !== '') {
        result += `<p>${currentParagraph}</p>`;
        currentParagraph = '';
      }
      result += line + '\n';
    }
  }
  
  // 处理最后一个段落
  if (currentParagraph !== '') {
    result += `<p>${currentParagraph}</p>`;
  }
  
  return result;
}

// HTML转义函数
function escapeHtml(text: string): string {
  const map: {[key: string]: string} = {
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    "'": '&#039;'
  };
  return text.replace(/[&<>"']/g, (m) => map[m]);
}

// 组件挂载时解析初始文本
onMounted(() => {
  // 设置默认示例文本
  markdownText.value = `# Markdown编辑器示例

这是一个**功能完整**的Markdown编辑器，支持以下语法：

## 标题
# 一级标题
## 二级标题
### 三级标题

## 文本样式
**粗体文本** 和 *斜体文本*

## 列表
### 无序列表
- 项目1
- 项目2
- 项目3

### 有序列表
1. 第一项
2. 第二项
3. 第三项

## 引用
> 这是一段引用文本
> 可以有多行

## 代码
### 内联代码
这是一段\`内联代码\`示例

### 代码块
\`\`\`javascript
function markdownParser(text) {
  // 解析Markdown文本
  return parsedHtml;
}
\`\`\`

## 链接和图片
[GitHub](https://github.com)

![图片示例](https://via.placeholder.com/300x200?text=Markdown+Editor)`;
  parseMarkdown();
});
</script>

<style scoped>
/* 基础样式 */
.markdown-editor-container {
  width: 100%;
  height: 100%;
  display: flex;
  flex-direction: column;
  background-color: #fff;
}

.editor-toolbar {
  padding: 15px;
  background-color: #f6f8fa;
  border-bottom: 1px solid #e1e4e8;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.editor-toolbar h2 {
  margin: 0;
  font-size: 18px;
  font-weight: 600;
  color: #24292e;
}

.size-controls {
  display: flex;
  gap: 20px;
}

.size-controls label {
  display: flex;
  flex-direction: column;
  font-size: 12px;
  color: #586069;
  gap: 5px;
}

.size-controls input[type="range"] {
  width: 100px;
}

.size-controls select {
  width: 120px;
  padding: 4px;
  border: 1px solid #d1d5da;
  border-radius: 3px;
  background-color: #fff;
}

.editor-content {
  flex: 1;
  padding: 20px;
  display: flex;
  gap: 20px;
  overflow: hidden;
}

.editor-content.vertical {
  flex-direction: column;
}

.editor-content.split {
  flex-wrap: wrap;
}

.editor-section,
.preview-section {
  display: flex;
  flex-direction: column;
}

.editor-section h3,
.preview-section h3 {
  margin: 0 0 10px 0;
  font-size: 14px;
  font-weight: 600;
  color: #24292e;
}

textarea {
  flex: 1;
  padding: 12px;
  border: 1px solid #d1d5da;
  border-radius: 6px;
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol";
  font-size: 14px;
  line-height: 1.5;
  resize: both;
  background-color: #fff;
  color: #24292e;
}

textarea:focus {
  outline: none;
  border-color: #0366d6;
  box-shadow: 0 0 0 3px rgba(3, 102, 214, 0.1);
}

.markdown-preview {
  flex: 1;
  padding: 12px;
  border: 1px solid #e1e4e8;
  border-radius: 6px;
  background-color: #fff;
  overflow-y: auto;
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol";
  line-height: 1.5;
}

/* GitHub风格的Markdown样式 */
:deep(pre) {
  background-color: #f6f8fa;
  padding: 16px;
  border-radius: 6px;
  overflow-x: auto;
  font-family: "SFMono-Regular", Consolas, "Liberation Mono", Menlo, Courier, monospace;
  font-size: 13px;
  line-height: 1.5;
}

:deep(code) {
  background-color: #f6f8fa;
  padding: 0.2em 0.4em;
  border-radius: 3px;
  font-family: "SFMono-Regular", Consolas, "Liberation Mono", Menlo, Courier, monospace;
  font-size: 85%;
  color: #e96900;
}

:deep(pre code) {
  background-color: transparent;
  padding: 0;
  color: inherit;
}

:deep(h1),
:deep(h2),
:deep(h3),
:deep(h4),
:deep(h5),
:deep(h6) {
  margin-top: 24px;
  margin-bottom: 16px;
  font-weight: 600;
  line-height: 1.25;
  color: #24292e;
}

:deep(h1) {
  padding-bottom: 0.3em;
  font-size: 2em;
  border-bottom: 1px solid #eaecef;
}

:deep(h2) {
  padding-bottom: 0.3em;
  font-size: 1.5em;
  border-bottom: 1px solid #eaecef;
}

:deep(h3) {
  font-size: 1.25em;
}

:deep(p) {
  margin-top: 0;
  margin-bottom: 16px;
  color: #24292e;
}

:deep(strong) {
  font-weight: 600;
  color: #24292e;
}

:deep(em) {
  font-style: italic;
  color: #24292e;
}

:deep(blockquote) {
  padding: 0 1em;
  color: #6a737d;
  border-left: 4px solid #dfe2e5;
  margin: 16px 0;
}

:deep(blockquote p) {
  color: #6a737d;
}

:deep(ul),
:deep(ol) {
  padding-left: 2em;
  margin-top: 0;
  margin-bottom: 16px;
  color: #24292e;
}

:deep(li) {
  margin-bottom: 4px;
}

:deep(a) {
  color: #0366d6;
  text-decoration: none;
}

:deep(a:hover) {
  text-decoration: underline;
}

:deep(img) {
  max-width: 100%;
  box-sizing: content-box;
  background-color: #fff;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .editor-content {
    flex-direction: column;
  }
  
  .editor-section,
  .preview-section {
    width: 100%;
  }
  
  .size-controls {
    flex-direction: column;
    gap: 10px;
  }
}
</style>