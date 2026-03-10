# Qt OpenCV 学习项目

一个最小的 Qt 6 Widgets 学习项目，用 OpenCV 读取/生成图片并展示。

## 依赖
- Qt 6.8.3（安装在 /home/zyh/Qt/6.8.3）
- OpenCV（通过 apt 安装）

## 构建
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/home/zyh/Qt/6.8.3/gcc_64
cmake --build build
```

## 运行
```bash
./build/QtOpenCVWebpViewer
```

## 目录结构

```mermaid
graph LR
    subgraph 项目架构
        MAIN["main.cpp<br/>程序入口"] --> MW["main_window.*<br/>主窗口 / 导航"]
        MW --> L01["01 生成并保存图片<br/>cv::imwrite"]
        MW --> L02["02 读取并显示图片<br/>cv::imread"]
        MW --> L03["03 窗口显示<br/>namedWindow"]
        MW --> L04["04 腐蚀与膨胀<br/>erode / dilate"]
        MW --> L05["05 边界提取<br/>absdiff"]
        MW --> L06["06 灰度变换<br/>Gamma 校正"]
        MW --> L07["07 直方图<br/>equalizeHist"]
        MW --> L08["08 截断<br/>threshold TRUNC"]
        MW --> L09["09 饱和度与颜色<br/>HSV / BGR 调色"]
        MW --> L10["10 反相<br/>bitwise_not"]
        MW --> L11["11 二值化<br/>threshold BINARY"]
        MW --> L12["12 对比度拉伸<br/>minMaxLoc + convertTo"]
        MW --> L13["13 色彩空间<br/>BGR / HSV / YCrCb / Lab"]
        MW --> L14["14 颜色阈值分割<br/>inRange / mask"]
        MW --> L15["15 分割后处理<br/>morphology / components"]
        MW --> L16["16 连通区域分析<br/>stats / filtering"]
        UTIL["mat_to_qimage.*<br/>Mat → QImage 转换"] -.->|"各课程共用"| MW
    end

    style MAIN fill:#4CAF50,color:#fff
    style MW fill:#2196F3,color:#fff
    style UTIL fill:#FF9800,color:#fff
```

### 课程学习路线图

```mermaid
graph TD
    L01["01 生成并保存图片"] --> L02["02 读取并显示图片"]
    L02 --> L03["03 窗口显示"]
    L03 --> L04["04 腐蚀与膨胀"]
    L04 --> L05["05 边界提取"]
    L05 --> L06["06 点运算-灰度变换"]

    L06 --> L07["07 直方图"]
    L06 --> L08["08 截断"]
    L06 --> L09["09 饱和度与颜色"]
    L06 --> L10["10 反相"]
    L06 --> L11["11 二值化"]
    L06 --> L12["12 对比度拉伸"]
    L06 --> L13["13 色彩空间-BGR HSV YCrCb Lab"]
    L06 --> L14["14 颜色阈值分割-inRange"]
    L06 --> L15["15 分割后处理-形态学与连通区域"]
    L15 --> L16["16 连通区域分析-connectedComponentsWithStats"]

    subgraph "基础篇"
        L01
        L02
        L03
    end

    subgraph "形态学篇"
        L04
        L05
    end

    subgraph "点运算篇（可并行学习）"
        L07
        L08
        L09
        L10
        L11
        L12
        L13
        L14
        L15
        L16
    end

    style L01 fill:#C8E6C9
    style L02 fill:#C8E6C9
    style L03 fill:#C8E6C9
    style L04 fill:#BBDEFB
    style L05 fill:#BBDEFB
    style L06 fill:#FFE0B2
    style L07 fill:#FFE0B2
    style L08 fill:#FFE0B2
    style L09 fill:#FFE0B2
    style L10 fill:#FFE0B2
    style L11 fill:#FFE0B2
    style L12 fill:#FFE0B2
    style L13 fill:#FFE0B2
    style L14 fill:#FFE0B2
    style L15 fill:#FFE0B2
    style L16 fill:#FFE0B2
```

### 文件列表
- main.cpp：入口
- main_window.*：主窗口（首页+导航）
- 01 生成并保存图片/：imwrite 子项目
- 02 读取并显示图片/：imread 子项目
- 03 窗口显示/：namedWindow 子项目
- 04 腐蚀与膨胀/：滑动条腐蚀/膨胀子项目
- 05 边界提取/：腐蚀边界提取子项目
- 06 点运算-灰度变换/：点运算灰度变换子项目
- 07 点运算-直方图/：点运算直方图子项目
- 08 点运算-截断/：点运算截断子项目
- 09 点运算-提升饱和度与颜色/：点运算颜色调整子项目
- 10 点运算-反相/：点运算反相子项目
- 11 点运算-二值化/：点运算二值化子项目
- 12 点运算-对比度拉伸/：点运算对比度拉伸子项目
- 13 色彩空间-BGR HSV YCrCb Lab/：色彩空间入门与用途对比子项目
- 14 颜色阈值分割-inRange/：颜色分割与 inRange 子项目
- 15 分割后处理-形态学与连通区域/：分割后处理与连通区域筛选子项目
- 16 连通区域分析-connectedComponentsWithStats/：连通区域标号、面积筛选与最大区域子项目
- mat_to_qimage.*：OpenCV 到 QImage 转换

### Lesson 13 补充说明

- lesson 13 现在不仅包含 `BGR / HSV / YCrCb / Lab` 的文字讲解，还带有多张原创 SVG 示意图。
- Qt 界面里可以直接预览这些示意图，包括 `HSV 圆锥`、`Hue 色相环`、`BGR 只增加 R`、`Lab 的 L/a/b`，以及“四种空间提亮对比”。
- 如果你想快速理解“为什么同样叫提亮，结果却不一样”，建议优先查看 lesson 13 文档中的“四种空间提亮对比”图，再配合界面里的“用途对比”按钮观察。
