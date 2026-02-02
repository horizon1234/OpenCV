cmake_minimum_required(VERSION 3.21)
project(QtOpenCVWebpViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 固定 Qt 6.8.3 安装路径
set(CMAKE_PREFIX_PATH "/home/zyh/Qt/6.8.3/gcc_64" CACHE PATH "Qt 6.8.3 prefix" FORCE)
set(Qt6_DIR "/home/zyh/Qt/6.8.3/gcc_64/lib/cmake/Qt6" CACHE PATH "Qt 6.8.3 CMake config" FORCE)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    main_window.cpp
    imwrite_lesson_widget.cpp
    imread_lesson_widget.cpp
    named_window_lesson_widget.cpp
    morphology_trackbar_lesson_widget.cpp
    erosion_boundary_lesson_widget.cpp
    mat_to_qimage.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        ${OpenCV_LIBS}
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
