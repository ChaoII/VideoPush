
project(core_helper)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)
# 添加头文件路径
include_directories(${CMAKE_SOURCE_DIR})


set(HEADERS
        appdata.h
        appinit.h
        base64helper.h
        customstyle.h
        iconhelper.h
        qthelper.h
)

set(SOURCES
        appdata.cpp
        appinit.cpp
        base64helper.cpp
        customstyle.cpp
        iconhelper.cpp
        qthelper.cpp
)

# 资源文件
set(RESOURCES)
list(APPEND RESOURCES qrc/image.qrc)
list(APPEND RESOURCES qrc/qm.qrc)
list(APPEND RESOURCES qrc/font.qrc)

# 编译警告配置
add_compile_options(-w)

# 添加大资源支持
add_definitions(-DQT_NO_DEBUG_OUTPUT)

# WASM支持
if(EMSCRIPTEN)
    list(APPEND HEADERS wasmhelper.h)
    list(APPEND SOURCES wasmhelper.cpp)
    list(APPEND RESOURCES qrc/wasm.qrc)
endif()

# 添加源文件和头文件
add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${RESOURCES})
# 链接Qt库
target_link_libraries(${PROJECT_NAME} Qt::Core Qt::Gui Qt::Widgets Qt::Network)
