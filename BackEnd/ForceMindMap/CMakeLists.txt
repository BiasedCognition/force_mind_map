cmake_minimum_required(VERSION 3.31)
project(ForceMindMap)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置包含目录
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Qt6 REQUIRED COMPONENTS Core WebEngineWidgets WebChannel Widgets Gui)

qt_add_executable(${PROJECT_NAME}
        main.cpp
        include/WebNodeManager.h
        include/MainWindow.h
        include/AppConfig.h
        src/MainWindow.cpp
        src/WebNodeManager.cpp
        src/AppConfig.cpp

)

qt_add_resources(${PROJECT_NAME} "resources"
        PREFIX "/"
        FILES
        resources/index.html
        resources/favicon.ico
        resources/d3.v7.min.js
        resources/mindmap.js
        # 可选的KaTeX字体文件，当前应用可能不需要
        # resources/assets/KaTeX_AMS-Regular-BQhdFMY1.woff2
        # 更多KaTeX字体文件可根据需要取消注释
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::WebEngineWidgets
        Qt6::WebChannel
        Qt6::Widgets
        Qt6::Gui
)