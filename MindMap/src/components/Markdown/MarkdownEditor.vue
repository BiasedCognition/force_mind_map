<template>
  <div class="markdown-editor-container">
    <div class="editor-toolbar">
      <h2>Markdown Editor</h2>
    </div>
    
    <div class="editor-content">
      <div class="preview-section">
        <h3>预览区</h3>
        <div 
          ref="markdownPreview"
          class="markdown-preview markdown-body"
          v-html="htmlContent"
        ></div>
      </div>
      
      <div class="editor-section">
        <h3>输入区</h3>
        <div class="editor-with-line-numbers">
          <div 
            ref="lineNumbersContainer"
            class="line-numbers"
          ></div>
          <textarea 
            ref="editorTextarea"
            v-model="markdownText" 
            @input="parseMarkdown"
            placeholder="在此输入Markdown文本...\n\n# 标题示例\n## 二级标题\n### 三级标题\n\n**粗体文本** 和 *斜体文本*\n\n- 列表项1\n- 列表项2\n\n> 引用文本\n\n`代码块`\n\n```javascript\nfunction example() {\n  console.log('代码块示例');\n}\n```\n\n[链接文本](https://example.com)\n\n![图片描述](https://via.placeholder.com/150)"
          ></textarea>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive } from 'vue';
import './github-markdown.css';
import katex from 'katex';
import 'katex/dist/katex.min.css';

const markdownText = ref('');
const htmlContent = ref('');
const editorTextarea = ref<HTMLTextAreaElement | null>(null);
const markdownPreview = ref<HTMLElement | null>(null);
const lineNumbersContainer = ref<HTMLElement | null>(null);
const cursorPosition = reactive({ line: 0, column: 0 });
const lastHighlightedLine = ref<number | null>(null);
const lastHighlightedElement = ref<Element | null>(null);

// 更新行号显示
  function updateLineNumbers() {
    if (!lineNumbersContainer.value || !editorTextarea.value) return;
    
    // 获取文本行数
    const linesCount = Math.max(1, markdownText.value.split('\n').length);
    
    // 清空并重建行号
    lineNumbersContainer.value.innerHTML = '';
    for (let i = 1; i <= linesCount; i++) {
      const lineNumberElement = document.createElement('div');
      lineNumberElement.className = 'line-number';
      if (i === cursorPosition.line) {
        lineNumberElement.classList.add('cursor-line');
      }
      lineNumberElement.textContent = i.toString();
      lineNumbersContainer.value.appendChild(lineNumberElement);
    }
    
    // 同步滚动位置
    if (lineNumbersContainer.value && editorTextarea.value) {
      lineNumbersContainer.value.scrollTop = editorTextarea.value.scrollTop;
    }
  }

  // 获取光标所在行号和列号
  function getCursorPosition() {
    if (!editorTextarea.value) return { line: 0, column: 0 };
    
    const textarea = editorTextarea.value;
    const text = textarea.value;
    const caretPosition = textarea.selectionStart;
    
    // 计算行号和列号
    const lines = text.substring(0, caretPosition).split('\n');
    const line = lines.length;
    const column = lines[lines.length - 1].length + 1;
    
    return { line, column };
  }

  // 高亮光标所在行
  function highlightCursorLine() {
    if (!editorTextarea.value || !lineNumbersContainer.value) return;
    
    const { line } = cursorPosition;
    
    // 移除之前高亮的行
    if (lastHighlightedLine.value && lastHighlightedLine.value !== line) {
      const oldLineElement = lineNumbersContainer.value.querySelector(`.line-number:nth-child(${lastHighlightedLine.value})`);
      if (oldLineElement) {
        oldLineElement.classList.remove('cursor-line');
      }
    }
    
    // 高亮当前行
    const currentLineElement = lineNumbersContainer.value.querySelector(`.line-number:nth-child(${line})`);
    if (currentLineElement) {
      currentLineElement.classList.add('cursor-line');
    }
    
    // 更新最后高亮的行号
    lastHighlightedLine.value = line;
  }

  // 高亮对应的HTML标签
  function highlightHtmlElement() {
    if (!markdownPreview.value) return;
    
    const { line } = cursorPosition;
    const preview = markdownPreview.value;
    
    // 移除之前高亮的元素
    if (lastHighlightedElement.value) {
      lastHighlightedElement.value.classList.remove('highlighted-element');
      lastHighlightedElement.value = null;
    }
    
    // 查找对应行的HTML元素
    const elementsWithLineRange = preview.querySelectorAll('[data-line-range]');
    let targetElement: Element | null = null;
    
    // 优先查找光标行号在开始行和结束行之间的元素
    elementsWithLineRange.forEach(element => {
      const lineRangeStr = element.getAttribute('data-line-range');
      if (lineRangeStr) {
        const [startLine, endLine] = lineRangeStr.split(',').map(Number);
        if (line > startLine && line < endLine) {
          targetElement = element;
        }
      }
    });
    
    // 如果没找到，查找开始行等于光标行号的元素
    if (!targetElement) {
      elementsWithLineRange.forEach(element => {
        const lineRangeStr = element.getAttribute('data-line-range');
        if (lineRangeStr) {
          const [startLine] = lineRangeStr.split(',').map(Number);
          if (startLine === line) {
            targetElement = element;
          }
        }
      });
    }
    
    // 高亮找到的元素
    if (targetElement) {
      // 添加类型断言帮助TypeScript正确识别类型
      const elementWithClassList = targetElement as HTMLElement;
      elementWithClassList.classList.add('highlighted-element');
      lastHighlightedElement.value = targetElement;
      return targetElement;
    }
    
    return null;
  }

// 行号信息类型
interface LineRange {
  startLine: number;
  endLine: number;
}

// 带行号信息的解析结果类型
interface ParsedWithLineInfo {
  html: string;
  lineRanges: { [htmlIndex: number]: LineRange };
}

// 自定义Markdown解析器 - 带行号标记
function parseMarkdown() {
  const text = markdownText.value;
  
  // 更新行号显示
  updateLineNumbers();
  
  if (!text.trim()) {
    htmlContent.value = '';
    return;
  }
  
  // 将文本按行分割，以便跟踪行号
  let lines = text.split('\n');
  let finalHtml = '';
  const allLineRanges: { [htmlIndex: number]: LineRange } = {};
  
  // 我们需要一种更复杂的解析策略，因为各种Markdown元素可能重叠
  // 1. 首先处理需要整体分析的块级元素
  
  // 1.1 解析代码块 (```)
  let currentText = lines.join('\n');
  
  // 1.2 处理代码块
  const codeBlockRegex = /```([\s\S]*?)```/g;
  let match: RegExpExecArray | null;
  let lastIndex = 0;
  let tempHtml = '';
  
  while ((match = codeBlockRegex.exec(currentText)) !== null) {
    // 添加匹配前的文本（需要进一步处理）
    const preMatchText = currentText.slice(lastIndex, match.index);
    if (preMatchText) {
      tempHtml += processInlineElements(preMatchText);
    }
    
    // 处理代码块
    const fullMatch = match[0];
    const code = match[1];
    
    // 提取语言标记
    const codeLines = code.split('\n');
    let language = '';
    let codeContent = '';
    
    if (codeLines.length > 0 && codeLines[0].trim()) {
      language = codeLines[0].trim();
      codeContent = codeLines.slice(1).join('\n');
    } else {
      codeContent = code;
    }
    
    // 查找代码块在原始文本中的行范围
    const startLine = countLines(currentText.substring(0, match.index)) + 1;
    const endLine = startLine + fullMatch.split('\n').length - 1;
    
    // 生成带行号标记的HTML
    const codeHtml = `<div class="highlight" data-line-range="${startLine},${endLine}"><pre><code class="language-${language}">${escapeHtml(codeContent).replace(/\n/g, '<br>')}</code></pre></div>`;
    tempHtml += codeHtml;
    
    lastIndex = match.index + match[0].length;
  }
  
  // 添加剩余文本
  if (lastIndex < currentText.length) {
    tempHtml += processInlineElements(currentText.slice(lastIndex));
  }
  
  // 2. 将处理后的文本按行分割，处理需要按行解析的元素
  const processedLines = tempHtml.split('\n');
  const finalProcessedLines: string[] = [];
  
  // 跟踪当前行号（在原始文本中的位置）
  let originalLineNumber = 1;
  
  for (let i = 0; i < lines.length; i++) {
    const originalLine = lines[i];
    
    // 检查是否已经被处理为代码块（通过检查data-line-range属性）
    let processed = false;
    
    for (let j = 0; j < finalProcessedLines.length; j++) {
      if (finalProcessedLines[j].includes(`data-line-range="${i+1},`)) {
        processed = true;
        break;
      }
    }
    
    if (!processed) {
      let processedLine = originalLine;
      
      // 检查并处理标题
      const headingMatch = /^(#{1,6}) (.*)$/.exec(originalLine);
      if (headingMatch) {
        const level = headingMatch[1].length;
        const content = headingMatch[2];
        processedLine = `<h${level} data-line-range="${i+1},${i+1}">${processInlineElements(content)}</h${level}>`;
      }
      // 检查并处理引用
      else if (originalLine.startsWith('> ')) {
        const content = originalLine.substring(2);
        processedLine = `<blockquote data-line-range="${i+1},${i+1}"><p>${processInlineElements(content)}</p></blockquote>`;
      }
      // 检查并处理无序列表
      else if (originalLine.startsWith('- ')) {
        const content = originalLine.substring(2);
        processedLine = `<li>${processInlineElements(content)}</li>`;
        // 检查前后行是否也是列表项，以确定是否需要添加ul标签
        let listStart = i;
        while (listStart > 0 && lines[listStart-1].startsWith('- ')) {
          listStart--;
        }
        
        if (i === listStart) {
          // 这是列表的开始
          processedLine = `<ul data-line-range="${listStart+1},${i+1}">${processedLine}`;
        }
        
        // 检查是否是列表的结束
        if (i === lines.length - 1 || !lines[i+1].startsWith('- ')) {
          processedLine += '</ul>';
        }
      }
      // 检查并处理有序列表
      else if (/^\d+\. /.test(originalLine)) {
        const match = /^(\d+)\. (.*)$/.exec(originalLine);
        if (match) {
          const number = parseInt(match[1]);
          const content = match[2];
          processedLine = `<li>${processInlineElements(content)}</li>`;
          
          // 检查前后行是否也是列表项，以确定是否需要添加ol标签
          let listStart = i;
          let listNumber = number;
          while (listStart > 0) {
            const prevMatch = /^(\d+)\. /.exec(lines[listStart-1]);
            if (prevMatch && parseInt(prevMatch[1]) === listNumber - 1) {
              listStart--;
              listNumber--;
            } else {
              break;
            }
          }
          
          if (i === listStart) {
            // 这是列表的开始
            processedLine = `<ol start="${listNumber}" data-line-range="${listStart+1},${i+1}">${processedLine}`;
          }
          
          // 检查是否是列表的结束
          if (i === lines.length - 1) {
            processedLine += '</ol>';
          } else {
            const nextMatch = /^(\d+)\. /.exec(lines[i+1]);
            if (!nextMatch || parseInt(nextMatch[1]) !== number + 1) {
              processedLine += '</ol>';
            }
          }
        }
      }
      // 处理普通文本行（可能是段落的一部分）
      else if (originalLine.trim() !== '') {
        processedLine = processInlineElements(originalLine);
      }
      
      finalProcessedLines.push(processedLine);
    }
    
    originalLineNumber++;
  }
  
  // 3. 将处理后的行重新组合成完整的HTML，并处理段落
  finalHtml = finalProcessedLines.join('\n');
  
  // 4. 解析段落（将连续的非标签行包装在<p>标签中）
  const paragraphRegex = /^(?!<(h[1-6]|blockquote|ul|ol|li|div|pre|code|p))([\s\S]*?)(?=\n\n|<(h[1-6]|blockquote|ul|ol|div|pre|code))/gm;
  finalHtml = finalHtml.replace(paragraphRegex, '<p>$2</p>');
  
  // 5. 处理任何剩余的独立行（可能是单行段落）
  const singleLineRegex = /^(?!<(h[1-6]|blockquote|ul|ol|li|div|pre|code|p))([^\n]+)$/gm;
  finalHtml = finalHtml.replace(singleLineRegex, '<p>$2</p>');
  
  // 6. 处理所有块级元素的行号信息
  // 确保所有生成的HTML标签都有正确的data-line-range属性
  
  // 最后，将结果赋值给htmlContent
  htmlContent.value = finalHtml;
}

// 处理内联元素的辅助函数
function processInlineElements(text: string): string {
  let result = text;
  
  // 处理内联代码 (`code`)
  result = parseInlineCode(result);
  
  // 处理粗体和斜体 (**粗体** 或 __粗体__, *斜体* 或 _斜体_)
  result = parseBoldAndItalic(result);
  
  // 处理LaTeX数学公式
  // 先解析块级公式 $$...$$
  result = parseBlockMath(result);
  // 再解析行内公式 $...$
  result = parseInlineMath(result);
  
  // 处理链接 ([链接文本](url))
  result = parseLinks(result);
  
  // 处理图片 (![图片描述](url))
  result = parseImages(result);
  
  return result;
}

// 解析代码块 - 带行号标记
function parseCodeBlocksWithLineNumbers(lines: string[]): ParsedWithLineInfo {
  let html = '';
  const lineRanges: { [htmlIndex: number]: LineRange } = {};
  let htmlIndex = 0;
  
  const text = lines.join('\n');
  const codeBlockRegex = /```([\s\S]*?)```/g;
  let match: RegExpExecArray | null;
  let lastIndex = 0;
  
  while ((match = codeBlockRegex.exec(text)) !== null) {
    // 添加匹配前的文本
    const preMatchText = text.slice(lastIndex, match.index);
    if (preMatchText) {
      html += preMatchText;
      htmlIndex += preMatchText.length;
    }
    
    const fullMatch = match[0];
    const code = match[1];
    
    // 提取语言标记
    const codeLines = code.split('\n');
    let language = '';
    let codeContent = '';
    
    if (codeLines.length > 0 && codeLines[0].trim()) {
      language = codeLines[0].trim();
      codeContent = codeLines.slice(1).join('\n');
    } else {
      codeContent = code;
    }
    
    // 查找代码块在原始文本中的行范围
    const startLine = countLines(text.substring(0, match.index)) + 1;
    const endLine = startLine + fullMatch.split('\n').length - 1;
    
    // 生成带行号标记的HTML
    const codeHtml = `<div class="highlight" data-line-range="${startLine},${endLine}"><pre><code class="language-${language}">${escapeHtml(codeContent).replace(/\n/g, '<br>')}</code></pre></div>`;
    
    lineRanges[htmlIndex] = { startLine, endLine };
    html += codeHtml;
    htmlIndex += codeHtml.length;
    
    lastIndex = match.index + match[0].length;
  }
  
  // 添加剩余文本
  if (lastIndex < text.length) {
    html += text.slice(lastIndex);
  }
  
  return { html, lineRanges };
}

// 解析代码块（兼容旧版本）
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

// 解析标题 - 带行号标记
function parseHeadingsWithLineNumbers(lines: string[]): ParsedWithLineInfo {
  let html = '';
  const lineRanges: { [htmlIndex: number]: LineRange } = {};
  let htmlIndex = 0;
  
  for (let i = 0; i < lines.length; i++) {
    let line = lines[i];
    let headingMatch = false;
    
    // 检查是否是标题行，从h1到h6
    for (let j = 1; j <= 6; j++) {
      const headingRegex = new RegExp(`^${'#'.repeat(j)} (.*?)$`);
      const match = headingRegex.exec(line);
      
      if (match) {
        const startLine = i + 1; // 行号从1开始
        const endLine = startLine;
        const headingText = match[1];
        
        // 添加带有行号标记的标题
        const headingHtml = `<h${j} data-line-range="${startLine},${endLine}">${headingText}</h${j}>`;
        lineRanges[htmlIndex] = { startLine, endLine };
        html += headingHtml;
        htmlIndex += headingHtml.length;
        
        headingMatch = true;
        break;
      }
    }
    
    // 如果不是标题，保持原行
    if (!headingMatch) {
      html += line + '\n';
      htmlIndex += line.length + 1; // +1 表示换行符
    }
  }
  
  return { html, lineRanges };
}

// 解析标题（兼容旧版本）
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

// 解析引用 - 带行号标记
function parseBlockquotesWithLineNumbers(lines: string[]): ParsedWithLineInfo {
  let html = '';
  const lineRanges: { [htmlIndex: number]: LineRange } = {};
  let htmlIndex = 0;
  
  let inBlockquote = false;
  let blockquoteStartLine = 0;
  let blockquoteContent = '';
  
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const blockquoteRegex = /^> (.*?)$/;
    const match = blockquoteRegex.exec(line);
    
    if (match) {
      // 进入引用或继续引用
      if (!inBlockquote) {
        inBlockquote = true;
        blockquoteStartLine = i + 1; // 行号从1开始
      }
      blockquoteContent += match[1] + '\n';
    } else if (inBlockquote) {
      // 引用结束
      const endLine = i;
      const blockquoteHtml = `<blockquote data-line-range="${blockquoteStartLine},${endLine}"><p>${blockquoteContent.trim()}</p></blockquote>\n`;
      
      lineRanges[htmlIndex] = { startLine: blockquoteStartLine, endLine };
      html += blockquoteHtml;
      htmlIndex += blockquoteHtml.length;
      
      inBlockquote = false;
      blockquoteContent = '';
      
      // 添加当前非引用行
      html += line + '\n';
      htmlIndex += line.length + 1;
    } else {
      // 非引用行
      html += line + '\n';
      htmlIndex += line.length + 1;
    }
  }
  
  // 处理最后一个引用（如果有）
  if (inBlockquote) {
    const endLine = lines.length;
    const blockquoteHtml = `<blockquote data-line-range="${blockquoteStartLine},${endLine}"><p>${blockquoteContent.trim()}</p></blockquote>`;
    
    lineRanges[htmlIndex] = { startLine: blockquoteStartLine, endLine };
    html += blockquoteHtml;
  }
  
  return { html, lineRanges };
}

// 解析引用（兼容旧版本）
function parseBlockquotes(text: string): string {
  const blockquoteRegex = /^> (.*?)$/gm;
  return text.replace(blockquoteRegex, '<blockquote><p>$1</p></blockquote>');
}

// 解析无序列表 - 带行号标记
function parseUnorderedListsWithLineNumbers(lines: string[]): ParsedWithLineInfo {
  let html = '';
  const lineRanges: { [htmlIndex: number]: LineRange } = {};
  let htmlIndex = 0;
  
  const ulRegex = /^- (.+?)$/;
  
  let inList = false;
  let listStartLine = 0;
  let listItems = '';
  
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const match = ulRegex.test(line);
    ulRegex.lastIndex = 0; // 重置正则表达式状态
    
    if (match) {
      // 进入列表或继续列表
      if (!inList) {
        inList = true;
        listStartLine = i + 1; // 行号从1开始
      }
      const listItemText = line.replace(/^- /, '');
      listItems += `<li>${listItemText}</li>`;
    } else if (inList) {
      // 列表结束
      const endLine = i;
      const listHtml = `<ul data-line-range="${listStartLine},${endLine}">${listItems}</ul>\n`;
      
      lineRanges[htmlIndex] = { startLine: listStartLine, endLine };
      html += listHtml;
      htmlIndex += listHtml.length;
      
      inList = false;
      listItems = '';
      
      // 添加当前非列表行
      html += line + '\n';
      htmlIndex += line.length + 1;
    } else {
      // 非列表行
      html += line + '\n';
      htmlIndex += line.length + 1;
    }
  }
  
  // 处理最后一个列表（如果有）
  if (inList) {
    const endLine = lines.length;
    const listHtml = `<ul data-line-range="${listStartLine},${endLine}">${listItems}</ul>`;
    
    lineRanges[htmlIndex] = { startLine: listStartLine, endLine };
    html += listHtml;
  }
  
  return { html, lineRanges };
}

// 解析无序列表（兼容旧版本）
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

// 解析有序列表 - 带行号标记
function parseOrderedListsWithLineNumbers(lines: string[]): ParsedWithLineInfo {
  let html = '';
  const lineRanges: { [htmlIndex: number]: LineRange } = {};
  let htmlIndex = 0;
  
  const olRegex = /^\d+\. (.+?)$/;
  
  let inList = false;
  let listStartLine = 0;
  let listItems = '';
  let expectedNumber = 1;
  
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const match = olRegex.exec(line);
    olRegex.lastIndex = 0; // 重置正则表达式状态
    
    if (match) {
      // 进入列表或继续列表
      if (!inList) {
        inList = true;
        listStartLine = i + 1; // 行号从1开始
        expectedNumber = parseInt(match[0].split('.')[0]);
      }
      const listItemText = match[1];
      listItems += `<li>${listItemText}</li>`;
    } else if (inList) {
      // 列表结束
      const endLine = i;
      const listHtml = `<ol start="${expectedNumber}" data-line-range="${listStartLine},${endLine}">${listItems}</ol>\n`;
      
      lineRanges[htmlIndex] = { startLine: listStartLine, endLine };
      html += listHtml;
      htmlIndex += listHtml.length;
      
      inList = false;
      listItems = '';
      
      // 添加当前非列表行
      html += line + '\n';
      htmlIndex += line.length + 1;
    } else {
      // 非列表行
      html += line + '\n';
      htmlIndex += line.length + 1;
    }
  }
  
  // 处理最后一个列表（如果有）
  if (inList) {
    const endLine = lines.length;
    const listHtml = `<ol start="${expectedNumber}" data-line-range="${listStartLine},${endLine}">${listItems}</ol>`;
    
    lineRanges[htmlIndex] = { startLine: listStartLine, endLine };
    html += listHtml;
  }
  
  return { html, lineRanges };
}

// 解析有序列表（兼容旧版本）
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

// 解析块级LaTeX公式 ($$...$$)
function parseBlockMath(text: string): string {
  const blockMathRegex = /\$\$([\s\S]*?)\$\$/g;
  return text.replace(blockMathRegex, (match, formula) => {
    try {
      console.log('解析LaTeX公式:', formula.trim());
      const result = katex.renderToString(formula.trim(), {
        throwOnError: false,
        displayMode: true,
        trust: true, // 允许更复杂的公式渲染
        output: 'html'
      });
      console.log('KaTeX渲染结果:', result);
      return result;
    } catch (error) {
      console.error('KaTeX渲染错误:', error);
      // 添加更详细的错误提示
      const errorMessage = error instanceof Error ? error.message : '未知错误';
      return `<div class="katex-error">
                <div class="error-formula">${escapeHtml(formula)}</div>
                <div class="error-message">渲染错误: ${escapeHtml(errorMessage)}</div>
              </div>`;
    }
  });
}

// 解析行内LaTeX公式 ($...$)
function parseInlineMath(text: string): string {
  const inlineMathRegex = /\$(.*?)\$/g;
  return text.replace(inlineMathRegex, (match, formula) => {
    // 避免解析代码块中的$符号
    if (match.includes('<code>') || match.includes('<pre>')) {
      return match;
    }
    try {
      return katex.renderToString(formula.trim(), {
        throwOnError: false,
        displayMode: false,
        trust: true, // 允许更复杂的公式渲染
        output: 'html'
      });
    } catch (error) {
      console.error('KaTeX渲染错误:', error);
      // 添加更详细的错误提示
      const errorMessage = error instanceof Error ? error.message : '未知错误';
      return `<span class="katex-error">
                <span class="error-formula">${escapeHtml(formula)}</span>
                <span class="error-message">[渲染错误: ${escapeHtml(errorMessage)}]</span>
              </span>`;
    }
  });
}

// 计算文本中的行数
function countLines(text: string): number {
  return text.split('\n').length - 1;
}

// 解析段落 - 带行号标记
function parseParagraphsWithLineNumbers(lines: string[]): ParsedWithLineInfo {
  let html = '';
  const lineRanges: { [htmlIndex: number]: LineRange } = {};
  let htmlIndex = 0;
  
  let currentParagraph = '';
  let paragraphStartLine = -1;
  
  for (let i = 0; i < lines.length; i++) {
    let line = lines[i];
    const trimmedLine = line.trim();
    
    // 检查是否已经是HTML标签
    const isHtmlTag = trimmedLine.startsWith('<') && trimmedLine.endsWith('>');
    
    // 如果是空行，且当前有段落内容，则闭合段落
    if (trimmedLine === '' && currentParagraph !== '') {
      const endLine = i;
      const paragraphHtml = `<p data-line-range="${paragraphStartLine},${endLine}">${currentParagraph}</p>`;
      
      lineRanges[htmlIndex] = { startLine: paragraphStartLine, endLine };
      html += paragraphHtml;
      htmlIndex += paragraphHtml.length;
      
      currentParagraph = '';
      paragraphStartLine = -1;
    } 
    // 如果行不是空行，且不是HTML标签，则添加到当前段落
    else if (trimmedLine !== '' && !isHtmlTag) {
      if (currentParagraph === '') {
        paragraphStartLine = i + 1; // 行号从1开始
      }
      if (currentParagraph !== '') currentParagraph += '\n';
      currentParagraph += trimmedLine;
    } 
    // 否则，直接添加该行（可能是HTML标签）
    else if (trimmedLine !== '') {
      if (currentParagraph !== '') {
        const endLine = i;
        const paragraphHtml = `<p data-line-range="${paragraphStartLine},${endLine}">${currentParagraph}</p>`;
        
        lineRanges[htmlIndex] = { startLine: paragraphStartLine, endLine };
        html += paragraphHtml;
        htmlIndex += paragraphHtml.length;
        
        currentParagraph = '';
        paragraphStartLine = -1;
      }
      html += line + '\n';
      htmlIndex += line.length + 1;
    }
  }
  
  // 处理最后一个段落
  if (currentParagraph !== '') {
    const endLine = lines.length;
    const paragraphHtml = `<p data-line-range="${paragraphStartLine},${endLine}">${currentParagraph}</p>`;
    
    lineRanges[htmlIndex] = { startLine: paragraphStartLine, endLine };
    html += paragraphHtml;
  }
  
  return { html, lineRanges };
}

// 解析段落（兼容旧版本）
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

![图片示例](https://via.placeholder.com/300x200?text=Markdown+Editor)

## LaTeX数学公式

### 语法提示
在Markdown编辑器中输入LaTeX公式时，请确保：
- 公式中的反斜杠（\\）需要正确转义
- 所有命令都使用正确的LaTeX语法
- 花括号、方括号等配对使用

### 行内公式
这是一个行内公式：$E=mc^2$，展示了相对论的质能方程。

### 块级公式
$$
sum_{i=1}^{n} i^2 = frac{n(n+1)(2n+1)}{6}
$$

$$
int_{a}^{b} f(x) \, dx
$$

$$
frac{partial u}{partial t} = alpha \nabla^2 u
$$

$$
begin{bmatrix} 1 & 2 \\ 3 & 4 \end{bmatrix}
$$

$$
begin{pmatrix} 1 & 2 \\ 3 & 4 \end{pmatrix}
$$`;
  
  // 手动添加反斜杠，避免在模板字符串中的转义问题
  markdownText.value = markdownText.value.replace(/\\/g, '\\\\')
    .replace(/(sum|frac|int|partial|alpha|nabla|begin|end)/g, '\\$1');
  parseMarkdown();
  
  // 添加光标位置跟踪
  if (editorTextarea.value) {
    const textarea = editorTextarea.value;
    
    // 处理光标位置变化
    const handleCursorMove = () => {
      const position = getCursorPosition();
      cursorPosition.line = position.line;
      cursorPosition.column = position.column;
      
      // 高亮光标所在行
      highlightCursorLine();
      
      // 高亮对应的HTML标签
      const highlightedElement = highlightHtmlElement();
      
      // 调整预览区位置
      if (highlightedElement && markdownPreview.value) {
        try {
          // 使用类型断言帮助TypeScript正确识别类型
          const rectElement = highlightedElement as HTMLElement;
          const targetRect = rectElement.getBoundingClientRect();
          const previewRect = markdownPreview.value.getBoundingClientRect();
          
          // 计算需要滚动的距离，确保高亮元素位于预览区的可见范围内
          const scrollDistance = targetRect.top - previewRect.top + markdownPreview.value.scrollTop - previewRect.height / 3;
          
          // 确保滚动位置在有效范围内
          markdownPreview.value.scrollTop = Math.max(0, Math.min(scrollDistance, markdownPreview.value.scrollHeight - previewRect.height));
        } catch (error) {
          console.error('预览区滚动调整出错:', error);
        }
      }
    };
    
    // 添加事件监听器
    textarea.addEventListener('click', handleCursorMove);
    textarea.addEventListener('keyup', handleCursorMove);
    textarea.addEventListener('keydown', handleCursorMove);
    textarea.addEventListener('scroll', () => {
      // 同步行号区域滚动
      if (lineNumbersContainer.value) {
        lineNumbersContainer.value.scrollTop = textarea.scrollTop;
      }
      // 更新光标位置
      handleCursorMove();
    });

    // 处理预览区点击事件 - 滚动到对应行并设置光标位置
    const handlePreviewClick = (event: MouseEvent) => {
      if (!editorTextarea.value) return;
      
      // 获取点击的元素，查找带有data-line-range属性的最近祖先
      let clickedElement = event.target as HTMLElement;
      let targetWithLineRange: HTMLElement | null = null;
      
      // 向上查找带有data-line-range属性的元素
      while (clickedElement && clickedElement !== markdownPreview.value) {
        if (clickedElement.getAttribute('data-line-range')) {
          targetWithLineRange = clickedElement;
          break;
        }
        clickedElement = clickedElement.parentElement as HTMLElement;
      }
      
      if (targetWithLineRange) {
        // 阻止事件冒泡，确保子元素的click覆盖父元素
        event.stopPropagation();
        
        const lineRangeStr = targetWithLineRange.getAttribute('data-line-range');
        if (lineRangeStr) {
          const [startLine, endLine] = lineRangeStr.split(',').map(Number);
          const targetLine = startLine; // 使用起始行作为目标行
          
          // 滚动到目标行并设置光标位置
          scrollToLineAndSetCursor(targetLine);
        }
      }
    };
    
    // 滚动到指定行并设置光标位置
    function scrollToLineAndSetCursor(lineNumber: number) {
      if (!editorTextarea.value) return;
      
      const textarea = editorTextarea.value;
      const lines = textarea.value.split('\n');
      
      // 确保行号在有效范围内
      const validLineNumber = Math.max(1, Math.min(lineNumber, lines.length));
      
      // 计算到目标行的字符位置
      let charPosition = 0;
      for (let i = 0; i < validLineNumber - 1; i++) {
        charPosition += lines[i].length + 1; // +1 用于换行符
      }
      
      // 计算滚动位置
      const lineHeight = 21; // 与CSS中设置的行高一致
      const textareaHeight = textarea.clientHeight;
      const centerScrollTop = (validLineNumber - 1) * lineHeight - textareaHeight / 2 + lineHeight / 2;
      
      // 设置滚动位置
      textarea.scrollTop = Math.max(0, Math.min(centerScrollTop, textarea.scrollHeight - textareaHeight));
      
      // 设置光标位置
      textarea.focus();
      textarea.setSelectionRange(charPosition, charPosition);
      
      // 更新光标位置和高亮显示
      cursorPosition.line = validLineNumber;
      cursorPosition.column = 1;
      highlightCursorLine();
      highlightHtmlElement();
    }
    
    // 为预览区添加点击事件监听器
    if (markdownPreview.value) {
      markdownPreview.value.addEventListener('click', handlePreviewClick);
    }

    // 初始化时设置光标位置
    handleCursorMove();
  }
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

/* LaTeX错误提示样式 */
.katex-error {
  background-color: #fee;
  border: 1px solid #fcc;
  border-radius: 4px;
  padding: 8px 12px;
  color: #c00;
  margin: 5px 0;
}

.katex-error .error-formula {
  font-family: monospace;
  background-color: #fff;
  padding: 4px 6px;
  border-radius: 3px;
  margin-bottom: 4px;
  word-break: break-all;
}

.katex-error .error-message {
  font-size: 12px;
  opacity: 0.9;
}

/* 行内公式错误样式 */
span.katex-error {
  display: inline-block;
  background-color: #fee;
  border: 1px solid #fcc;
  padding: 2px 6px;
}

span.katex-error .error-formula {
  display: inline;
  background-color: transparent;
  padding: 0;
  margin: 0;
}

span.katex-error .error-message {
  font-size: 11px;
  margin-left: 4px;
}

.editor-toolbar {
  padding: 15px;
  background-color: #f6f8fa;
  border-bottom: 1px solid #e1e4e8;
  display: flex;
  justify-content: center;
  align-items: center;
}

.editor-toolbar h2 {
  margin: 0;
  font-size: 18px;
  font-weight: 600;
  color: #24292e;
}

.editor-content {
  flex: 1;
  padding: 20px;
  display: flex;
  gap: 20px;
  overflow: hidden;
}

.editor-section,
.preview-section {
  display: flex;
  flex-direction: column;
  width: 50%;
  height: 100%;
}

.editor-section h3,
.preview-section h3 {
  margin: 0 0 10px 0;
  font-size: 14px;
  font-weight: 600;
  color: #24292e;
}

.editor-with-line-numbers {
  display: flex;
  flex: 1;
  height: calc(100% - 40px);
  border: 1px solid #d1d5da;
  border-radius: 6px;
  overflow: hidden;
}

.line-numbers {
  width: 50px;
  padding: 12px 8px;
  background-color: #f6f8fa;
  border-right: 1px solid #e1e4e8;
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol";
  font-size: 14px;
  line-height: 1.5;
  text-align: right;
  color: #586069;
  user-select: none;
  overflow-y: hidden;
}

.line-number {
  height: 21px; /* 与textarea行高匹配 */
  line-height: 1.5;
  transition: background-color 0.2s ease;
}

/* 光标所在行高亮样式 */
.line-number.cursor-line {
  background-color: rgba(0, 123, 255, 0.1);
  font-weight: 600;
  color: #0366d6;
  border-right: 3px solid #0366d6;
  margin-right: -8px;
  padding-right: 5px;
}

textarea {
  flex: 1;
  padding: 12px;
  border: none;
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol";
  font-size: 14px;
  line-height: 1.5;
  resize: none;
  background-color: #fff;
  color: #24292e;
  outline: none;
  overflow-y: auto;
  background-image: linear-gradient(transparent 0px, transparent calc(1.5em - 1px), #f0f0f0 calc(1.5em - 1px), #f0f0f0 1.5em);
  background-size: 100% 1.5em;
  background-attachment: local;
}

/* textarea光标行高亮 */
:deep(.cursor-line-highlight) {
  background-color: rgba(0, 123, 255, 0.05);
  transition: background-color 0.2s ease;
}

.editor-with-line-numbers:focus-within {
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
  width: 100%;
  height: calc(100% - 40px);
}

/* GitHub风格的Markdown样式 */

/* 预览区HTML标签hover样式 - 确保子元素的hover覆盖父元素 */
.markdown-preview :deep([data-line-range]) {
  transition: background-color 0.2s ease;
  cursor: pointer;
}

.markdown-preview :deep([data-line-range]):hover {
  background-color: rgba(0, 123, 255, 0.12) !important;
  transform: translateX(2px);
}

/* 确保子元素的hover覆盖父元素 */
.markdown-preview :deep([data-line-range] *) {
  cursor: pointer;
}

.markdown-preview :deep([data-line-range] *:hover) {
  background-color: inherit !important;
}

/* 特定元素的hover样式微调 */
.markdown-preview :deep(h1[data-line-range]):hover,
.markdown-preview :deep(h2[data-line-range]):hover,
.markdown-preview :deep(h3[data-line-range]):hover,
.markdown-preview :deep(h4[data-line-range]):hover,
.markdown-preview :deep(h5[data-line-range]):hover,
.markdown-preview :deep(h6[data-line-range]):hover {
  padding-left: 4px;
  margin-left: -4px;
}

.markdown-preview :deep(blockquote[data-line-range]):hover {
  padding-left: 14px;
  margin-left: -14px;
}

/* 预览区HTML标签高亮样式 */
.markdown-preview :deep(.highlighted-element) {
  background-color: rgba(0, 123, 255, 0.08) !important;
  border-left: 3px solid #0366d6 !important;
  padding-left: 8px !important;
  margin-left: -11px !important;
  transition: all 0.2s ease !important;
}

/* 特定元素的高亮样式调整 */
.markdown-preview :deep(p.highlighted-element),
.markdown-preview :deep(blockquote.highlighted-element),
.markdown-preview :deep(ul.highlighted-element),
.markdown-preview :deep(ol.highlighted-element) {
  padding-left: 15px !important;
  margin-left: -18px !important;
}
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
    height: 50%;
  }
}
</style>