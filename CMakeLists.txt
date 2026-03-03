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
    "lesson01_imwrite/imwrite_lesson_widget.cpp"
    "lesson02_imread/imread_lesson_widget.cpp"
    "lesson03_named_window/named_window_lesson_widget.cpp"
    "lesson04_morphology/morphology_trackbar_lesson_widget.cpp"
    "lesson05_erosion_boundary/erosion_boundary_lesson_widget.cpp"
    "lesson06_point_gray_transform/point_gray_transform_lesson_widget.cpp"
    "lesson07_point_histogram/point_histogram_lesson_widget.cpp"
    "lesson08_point_truncation/point_truncation_lesson_widget.cpp"
    "lesson09_point_color_adjust/point_color_adjust_lesson_widget.cpp"
    "lesson10_point_invert/point_invert_lesson_widget.cpp"
    "lesson11_point_threshold/point_threshold_lesson_widget.cpp"
    "lesson12_point_contrast_stretch/point_contrast_stretch_lesson_widget.cpp"
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
        COMMENT "Organizing runtime DLLs to dlls folder"
    )
endif()
