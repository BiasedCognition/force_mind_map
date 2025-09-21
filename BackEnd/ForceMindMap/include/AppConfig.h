#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>

/**
 * AppConfig 类 - 提供应用程序的配置选项
 * 主要用于配置前端资源的加载方式
 */
class AppConfig {
public:
    /**
     * 设置是否使用开发模式
     * @param useDevMode true表示使用开发模式（加载Vue开发服务器），false表示使用生产模式（加载构建后的文件）
     */
    static void setUseDevMode(bool useDevMode) {
        m_useDevMode = useDevMode;
    }

    /**
     * 获取是否使用开发模式
     * @return true表示使用开发模式，false表示使用生产模式
     */
    static bool useDevMode() {
        return m_useDevMode;
    }

    /**
     * 获取前端资源的URL
     * 根据配置返回开发服务器URL或构建后文件的本地路径
     * @return 前端资源的URL字符串
     */
    static QString getFrontendUrl() {
        if (m_useDevMode) {
            // 开发模式：加载Vue开发服务器
            return "http://localhost:5173";
        } else {
            // 生产模式：加载test1目录下构建后的前端资源
            return "file:///d:/MindMap/MindMap/test1/dist/index.html";
        }
    }

private:
    // 是否使用开发模式的标志
    static bool m_useDevMode;
};

#endif // APPCONFIG_H