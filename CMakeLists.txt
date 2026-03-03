cmake_minimum_required(VERSION 3.21)
project(QtOpenCVWebpViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/run_viewer.bat.in
        ${CMAKE_BINARY_DIR}/run_viewer.bat
        @ONLY
    )
endif()
# 固定 Qt 6.8.3 安装路径（允许命令行覆盖）
if (NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "/opt/Qt6.8.3/6.8.3/gcc_64" CACHE PATH "Qt 6.8.3 prefix")
endif()
if (NOT DEFINED Qt6_DIR)
    set(Qt6_DIR "/opt/Qt6.8.3/6.8.3/gcc_64/lib/cmake/Qt6" CACHE PATH "Qt 6.8.3 CMake config")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    main_window.cpp
    "01 生成并保存图片/imwrite_lesson_widget.cpp"
    "02 读取并显示图片/imread_lesson_widget.cpp"
    "03 窗口显示/named_window_lesson_widget.cpp"
    "04 腐蚀与膨胀/morphology_trackbar_lesson_widget.cpp"
    "05 边界提取/erosion_boundary_lesson_widget.cpp"
    "06 点运算-灰度变换/point_gray_transform_lesson_widget.cpp"
    "07 点运算-直方图/point_histogram_lesson_widget.cpp"
    "08 点运算-截断/point_truncation_lesson_widget.cpp"
    "09 点运算-提升饱和度与颜色/point_color_adjust_lesson_widget.cpp"
    "10 点运算-反相/point_invert_lesson_widget.cpp"
    "11 点运算-二值化/point_threshold_lesson_widget.cpp"
    "12 点运算-对比度拉伸/point_contrast_stretch_lesson_widget.cpp"
    mat_to_qimage.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        ${OpenCV_LIBS}
)

# Copy the executable to the project root for easy access.
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/run_viewer.bat
            ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/cat.jpg
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/1JPEG.webp
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/cat.jpg
        ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/1JPEG.webp
        ${CMAKE_SOURCE_DIR}
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

    find_program(
        WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt windeployqt6
        HINTS
            "${Qt6_DIR}/../../../bin"
            "C:/msys64/ucrt64/bin"
    )

    if (WINDEPLOYQT_EXECUTABLE)
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE}
                    --release
                    --compiler-runtime
                    --no-translations
                    $<TARGET_FILE:${PROJECT_NAME}>
            COMMENT "Deploying Qt runtime with windeployqt"
        )
    endif()

    get_filename_component(OPENCV_PREFIX "${OpenCV_DIR}/../../.." ABSOLUTE)
    set(OPENCV_BIN_DIR "${OPENCV_PREFIX}/bin")
    file(GLOB OPENCV_RUNTIME_DLLS "${OPENCV_BIN_DIR}/libopencv*.dll")

    if (OPENCV_RUNTIME_DLLS)
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${OPENCV_RUNTIME_DLLS}
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copying OpenCV runtime DLLs"
        )
    endif()

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DTARGET_DIR=$<TARGET_FILE_DIR:${PROJECT_NAME}>
            -P ${CMAKE_SOURCE_DIR}/cmake/organize_dlls.cmake
        COMMAND ${CMAKE_COMMAND}
            -DTARGET_DIR=${CMAKE_SOURCE_DIR}
            -P ${CMAKE_SOURCE_DIR}/cmake/organize_dlls.cmake
        COMMENT "Organizing runtime DLLs to dlls folder"
    )
endif()
