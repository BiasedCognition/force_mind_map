cmake_minimum_required(VERSION 3.16)

# 设置项目名称和版本
project(WebEngineTest VERSION 1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt包
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets WebEngineWidgets WebChannel)

# 如果找不到Qt6，尝试使用Qt5
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Widgets WebEngineWidgets WebChannel)
endif()

# 添加可执行文件
add_executable(WebEngineTest WebEngineTest.cpp)

# 链接Qt库
target_link_libraries(WebEngineTest PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::WebEngineWidgets
    Qt::WebChannel
)

# 自动处理MOC
set_target_properties(WebEngineTest PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# 如果是Windows平台，设置WIN32_EXECUTABLE属性，避免显示控制台窗口
if(WIN32)
    set_target_properties(WebEngineTest PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()