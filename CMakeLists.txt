cmake_minimum_required(VERSION 3.20)
project(VideoPush LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH "E:/develop/qt/6.7.1/msvc2019_64/lib/cmake")

option(WITH_NETPUSH "using netpush module" ON)
option(WITH_FILEPUSH "using filepush module" ON)
option(WITH_IMAGEPUSH "using imagepush module" ON)
option(WITH_OPENGLX "using openglx module" ON)


if (WIN32)
    set(FFMPEG_INCLUDE_DIRS "E:/develop/ffmpeg/include")
    set(FFMPEG_LIBRARY_DIRS "E:/develop/ffmpeg/lib")
else ()
    set(FFMPEG_INCLUDE_DIRS "/usr/local/Cellar/ffmpeg/6.1.1_2/include")
    set(FFMPEG_LIBRARY_DIRS "/usr/local/Cellar/ffmpeg/6.1.1_2/lib")
    # 其他系统
endif ()

if (WITH_NETPUSH)
    add_definitions(-Dnetpush)
endif ()

if (WITH_FILEPUSH)
    add_definitions(-Dfilepush)
endif ()

if (WITH_IMAGEPUSH)
    add_definitions(-Dimagepush)
endif ()

add_definitions(-Dffmpeg)
add_definitions(-Dffmpegdevice)
add_definitions(-Dvideoffmpeg)
add_definitions(-Dmultimedia)
add_definitions(-Daudiox)
add_definitions(-Dmultimedia5)
add_definitions(-Dopenglx)


set(CORE_VIDEOOPENGL)

if (WITH_OPENGLX)
    add_definitions(-Dopenglx)
endif ()


find_package(Qt6 COMPONENTS Core Widgets
        Network Core5Compat
        OpenGLWidgets Multimedia REQUIRED)
include_directories(${CMAKE_SOURCE_DIR})


aux_source_directory(app APP)
aux_source_directory(core_audio CORE_AUDIO)
aux_source_directory(core_helper CORE_HELPER)
aux_source_directory(core_video CORE_VIDEO)
aux_source_directory(core_videobase CORE_VIDEOBASE)
aux_source_directory(core_videoffmpeg CORE_VIDEOFFMPEG)
aux_source_directory(core_videohelper CORE_VIDEOHELPER)
aux_source_directory(core_videoopengl CORE_VIDEOOPENGL)
aux_source_directory(core_videopush CORE_VIDEOPUSH)
aux_source_directory(form FORM)
aux_source_directory(core_videopush/http_parser HTTPPARSER)

include_directories(app)
include_directories(core_audio)
include_directories(core_helper)
include_directories(core_video)
include_directories(core_videobase)
include_directories(core_videohelper)
include_directories(core_videoffmpeg)
include_directories(core_videoopengl)
include_directories(core_videopush)
include_directories(form)




set(SOURCE
        ${APP}
        ${CORE_AUDIO}
        ${CORE_HELPER}
        ${CORE_VIDEO}
        ${CORE_VIDEOBASE}
        ${CORE_VIDEOFFMPEG}
        ${CORE_VIDEOHELPER}
        ${CORE_VIDEOOPENGL}
        ${CORE_VIDEOPUSH}
        ${FORM}
        ${HTTPPARSER}
        main.cpp
        head.h
        main.qrc
)

set(FFMPEG_LIBRARIES
        avdevice
        avfilter
        avformat
        avcodec
        swresample
        swscale
        avutil)

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_directories(${PROJECT_NAME} PUBLIC ${FFMPEG_LIBRARY_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Qt::Widgets Qt::Core Qt::OpenGLWidgets ${FFMPEG_LIBRARIES} Qt::Network Qt::Core5Compat Qt::Multimedia )


if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
