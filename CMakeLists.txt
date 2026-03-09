cmake_minimum_required(VERSION 3.21)
project(QtOpenCVWebpViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_OBJECT_PATH_MAX 128)
endif()

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

find_package(Qt6 REQUIRED COMPONENTS Widgets SvgWidgets)
find_package(OpenCV REQUIRED)

set(LESSON_SOURCES
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
    "13 色彩空间-BGR HSV YCrCb Lab/color_space_lesson_widget.cpp"
    "14 颜色阈值分割-inRange/color_threshold_lesson_widget.cpp"
    "15 分割后处理-形态学与连通区域/segmentation_postprocess_lesson_widget.cpp"
    "16 连通区域分析-connectedComponentsWithStats/connected_components_lesson_widget.cpp"
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(WIN_SRC_DIR "${CMAKE_BINARY_DIR}/win_src")
    configure_file(
        ${CMAKE_SOURCE_DIR}/mat_to_qimage.h
        ${WIN_SRC_DIR}/mat_to_qimage.h
        COPYONLY
    )

    function(copy_lesson_sources chinese_dir ascii_dir out_var)
        file(MAKE_DIRECTORY "${WIN_SRC_DIR}/${ascii_dir}")
        file(GLOB lesson_files CONFIGURE_DEPENDS
            "${CMAKE_SOURCE_DIR}/${chinese_dir}/*.cpp"
            "${CMAKE_SOURCE_DIR}/${chinese_dir}/*.h"
        )

        set(local_cpp_files)
        foreach(src_file IN LISTS lesson_files)
            get_filename_component(file_name "${src_file}" NAME)
            set(dst_file "${WIN_SRC_DIR}/${ascii_dir}/${file_name}")
            configure_file("${src_file}" "${dst_file}" COPYONLY)
            if (file_name MATCHES "\\.cpp$")
                list(APPEND local_cpp_files "${dst_file}")
            endif()
        endforeach()

        set(${out_var} ${local_cpp_files} PARENT_SCOPE)
    endfunction()

    copy_lesson_sources("01 生成并保存图片" "lesson01" LESSON01_CPP)
    copy_lesson_sources("02 读取并显示图片" "lesson02" LESSON02_CPP)
    copy_lesson_sources("03 窗口显示" "lesson03" LESSON03_CPP)
    copy_lesson_sources("04 腐蚀与膨胀" "lesson04" LESSON04_CPP)
    copy_lesson_sources("05 边界提取" "lesson05" LESSON05_CPP)
    copy_lesson_sources("06 点运算-灰度变换" "lesson06" LESSON06_CPP)
    copy_lesson_sources("07 点运算-直方图" "lesson07" LESSON07_CPP)
    copy_lesson_sources("08 点运算-截断" "lesson08" LESSON08_CPP)
    copy_lesson_sources("09 点运算-提升饱和度与颜色" "lesson09" LESSON09_CPP)
    copy_lesson_sources("10 点运算-反相" "lesson10" LESSON10_CPP)
    copy_lesson_sources("11 点运算-二值化" "lesson11" LESSON11_CPP)
    copy_lesson_sources("12 点运算-对比度拉伸" "lesson12" LESSON12_CPP)
    copy_lesson_sources("13 色彩空间-BGR HSV YCrCb Lab" "lesson13" LESSON13_CPP)
    copy_lesson_sources("14 颜色阈值分割-inRange" "lesson14" LESSON14_CPP)
    copy_lesson_sources("15 分割后处理-形态学与连通区域" "lesson15" LESSON15_CPP)
    copy_lesson_sources("16 连通区域分析-connectedComponentsWithStats" "lesson16" LESSON16_CPP)

    set(LESSON_SOURCES
        ${LESSON01_CPP}
        ${LESSON02_CPP}
        ${LESSON03_CPP}
        ${LESSON04_CPP}
        ${LESSON05_CPP}
        ${LESSON06_CPP}
        ${LESSON07_CPP}
        ${LESSON08_CPP}
        ${LESSON09_CPP}
        ${LESSON10_CPP}
        ${LESSON11_CPP}
        ${LESSON12_CPP}
        ${LESSON13_CPP}
        ${LESSON14_CPP}
        ${LESSON15_CPP}
        ${LESSON16_CPP}
    )
endif()

add_executable(${PROJECT_NAME}
    main.cpp
    main_window.cpp
    ${LESSON_SOURCES}
    mat_to_qimage.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        Qt6::SvgWidgets
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
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_BINARY_DIR}/run_viewer.bat
                ${CMAKE_SOURCE_DIR}
    )

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
