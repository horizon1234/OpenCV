# 课程 02：读取并显示图片（cv::imread）

> 适合人群：零基础初学者 | 预计阅读时间：25 分钟

---

## 零、预备知识

### 图片文件是怎么存在硬盘上的？

图片文件本质上就是一堆数字，按照特定的格式规则排列后保存在硬盘上。当你用 `imread` 读取一张图片时，OpenCV 做了这些事：

```
硬盘上的文件                           内存中的 cv::Mat
┌──────────────────┐                 ┌──────────────────┐
│ cat.jpg           │    imread()    │ 二维数组          │
│ [文件头][压缩数据] │ ───────────▶  │ 480 行 × 640 列   │
│ 总大小 ~50KB      │    解码        │ 每像素 3 字节     │
└──────────────────┘                 │ 总大小 ~921 KB    │
                                     └──────────────────┘
  有损压缩的 JPEG                      未压缩的原始像素数据
```

> 你发现了吗？**文件大小 ≠ 图片数据大小**。一张 50KB 的 JPEG 文件，解压后在内存中可能占用近 1MB。这是因为 JPEG 使用了**有损压缩**。

---

## 一、本课目标

本课你将学会：

1. **用 `cv::imread` 从文件读取图片**到内存
2. 理解 **imread 的读取模式**（彩色 / 灰度 / 不变）
3. 理解**行步长（step）**的概念及其重要性
4. 看到**错误的 step 会导致什么后果**（图像错位）

```
整体流程：

  ┌────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────┐
  │ 硬盘上的    │───▶│ cv::imread   │───▶│ cv::Mat      │───▶│ 界面显示  │
  │ cat.jpg    │    │ 解码图片文件  │    │ 内存中的像素  │    │ QLabel   │
  └────────────┘    └──────────────┘    └──────────────┘    └──────────┘
```

---

## 二、cv::imread 详解

### 2.1 函数签名

```cpp
cv::Mat cv::imread(const std::string& filename, 
                   int flags = cv::IMREAD_COLOR);
```

| 参数 | 含义 | 示例 |
|------|------|------|
| `filename` | 图片文件路径 | `"cat.jpg"`, `"/home/user/photo.png"` |
| `flags` | 读取模式（决定返回几个通道） | `IMREAD_COLOR`, `IMREAD_GRAYSCALE` |
| 返回值 | 读取到的图片（Mat），失败则返回空 Mat | `if (img.empty()) ...` |

### 2.2 读取模式详解

```cpp
// 模式 1：彩色（默认）—— 返回 3 通道 BGR
cv::Mat color = cv::imread("cat.jpg", cv::IMREAD_COLOR);
// color.channels() == 3

// 模式 2：灰度 —— 返回 1 通道灰度图
cv::Mat gray = cv::imread("cat.jpg", cv::IMREAD_GRAYSCALE);
// gray.channels() == 1

// 模式 3：原样 —— 保留原始通道数（可能含 Alpha）
cv::Mat original = cv::imread("cat.jpg", cv::IMREAD_UNCHANGED);
// 如果是 PNG 带透明度，channels() == 4
```

用一张图理解三种模式：

```
原始图片（假设是 PNG，4 通道 BGRA）：

  IMREAD_COLOR       IMREAD_GRAYSCALE     IMREAD_UNCHANGED
  ┌──────────┐       ┌──────────┐         ┌──────────┐
  │ B G R    │       │ Gray     │         │ B G R A  │
  │ (3通道)  │       │ (1通道)  │         │ (4通道)  │
  └──────────┘       └──────────┘         └──────────┘
  丢弃 Alpha          彩色→灰度            完全保留
  如有 Alpha 则丢弃    加权平均             原汁原味
```

**imread 读取模式完整参考表**：

| 标志 | 数值 | 输出通道数 | 输出类型 | 效果 |
|------|------|----------|---------|------|
| `IMREAD_COLOR` | 1 | 3 | CV_8UC3 | 强制转 BGR（默认） |
| `IMREAD_GRAYSCALE` | 0 | 1 | CV_8UC1 | 强制转灰度 |
| `IMREAD_UNCHANGED` | -1 | 原始 | 原始 | 保留所有通道和位深 |
| `IMREAD_ANYDEPTH` | 2 | — | 原始位深 | 保留 16/32 位深度 |
| `IMREAD_ANYCOLOR` | 4 | 原始 | CV_8U | 保留原始通道数 |
| `IMREAD_REDUCED_COLOR_2` | 17 | 3 | CV_8UC3 | 尺寸缩小 1/2 |
| `IMREAD_REDUCED_COLOR_4` | 33 | 3 | CV_8UC3 | 尺寸缩小 1/4 |

> 常用的就前三个，其他了解即可。

**灰度转换公式**：

$$
Gray = 0.299 \times R + 0.587 \times G + 0.114 \times B
$$

为什么绿色权重最大？因为人眼对绿色最敏感（回忆课程 06 的韦伯-费希纳定律）。

### 2.3 路径问题（初学者最常犯的错）

```
❌ 常见错误：imread 返回空 Mat

  原因 1：文件路径写错了
    cv::imread("Cat.jpg")   ← Linux 区分大小写！应该是 "cat.jpg"
    
  原因 2：工作目录不对
    你以为在项目根目录运行，实际在 build/ 目录运行
    
  原因 3：文件根本不存在
    忘记把 cat.jpg 复制到正确位置

✅ 排查方法：
    #include <filesystem>
    std::cout << "当前目录: " << std::filesystem::current_path() << std::endl;
    std::cout << "文件存在: " << std::filesystem::exists("cat.jpg") << std::endl;
```

```
路径类型：

  相对路径：  "cat.jpg"          → 相对于程序运行时的工作目录
             "../data/cat.jpg"  → 上一级目录的 data 子目录

  绝对路径：  "/home/user/images/cat.jpg"  → 完整路径，不受工作目录影响

  建议：开发阶段用相对路径，部署时用绝对路径
```

---

## 三、行步长（Step）—— 本课核心难点

### 3.1 什么是 Step？

**Step**（也叫 stride，步长）是指**一行像素数据在内存中占多少字节**。

```
一张 4 列 × 3 行的灰度图（每像素 1 字节）：

  理想情况（无填充）：
  行 0: [P00] [P01] [P02] [P03]          ← step = 4 字节
  行 1: [P10] [P11] [P12] [P13]          ← step = 4 字节
  行 2: [P20] [P21] [P22] [P23]          ← step = 4 字节

  有填充的情况（为了 8 字节对齐）：
  行 0: [P00] [P01] [P02] [P03] [##] [##] [##] [##]   ← step = 8 字节
  行 1: [P10] [P11] [P12] [P13] [##] [##] [##] [##]   ← step = 8 字节
  行 2: [P20] [P21] [P22] [P23] [##] [##] [##] [##]   ← step = 8 字节

  [##] = 填充字节（padding），不包含有效数据
```

**常见图像尺寸的 step 参考表**：

| 图像尺寸 | 类型 | 通道 | 像素字节/行 | 典型 step | 内存占用 |
|---------|------|------|-----------|----------|---------|
| 640×480 | CV_8UC1 | 1 | 640 | 640 | 300 KB |
| 640×480 | CV_8UC3 | 3 | 1920 | 1920 | 900 KB |
| 1920×1080 | CV_8UC1 | 1 | 1920 | 1920 | 1.98 MB |
| 1920×1080 | CV_8UC3 | 3 | 5760 | 5760 | 5.93 MB |
| 3840×2160 | CV_8UC3 | 3 | 11520 | 11520 | 23.7 MB |
| 640×480 | CV_8UC4 | 4 | 2560 | 2560 | 1.17 MB |

> **公式**：`step ≥ cols × channels × sizeof(uchar)`，`内存 ≈ step × rows`。
> OpenCV 的 `cv::Mat` 通常不做额外对齐，所以 `step == cols × channels`。

### 3.2 为什么需要内存对齐？

```
CPU 读取内存的方式：

  CPU 不是一个字节一个字节读的，而是一次读 4 / 8 / 16 / 32 字节
  （取决于 CPU 架构和指令集）

  对齐的地址：  0x1000, 0x1008, 0x1010 ...  ← 一次读取搞定
  不对齐的地址：0x1003                       ← 可能需要两次读取！

  所以 step 有时会比 cols × channels 大，多出来的是填充字节
```

### 3.3 如果 step 搞错了会怎样？（**本课重点实验**）

本课的代码故意用错误的 step 构造一个 `QImage`，让你直观感受后果：

```cpp
// 正确的 step
const int correctStep = image.cols;  // 灰度图：每行 cols 个字节

// 故意做错的 step（多了 16 字节）
const int wrongStep = image.cols + 16;
```

```
用正确 step 读取像素：

  内存：[A B C D] [E F G H] [I J K L]
  step = 4
  
  行 0: A B C D    ← 从 0 开始，读 4 个
  行 1: E F G H    ← 从 4 开始，读 4 个   → 正确 ✓
  行 2: I J K L    ← 从 8 开始，读 4 个   → 正确 ✓

用错误 step 读取像素：

  内存：[A B C D] [E F G H] [I J K L]
  step = 6（故意多了 2）
  
  行 0: A B C D    ← 从 0 开始，读 4 个   → OK
  行 1: G H I J    ← 从 6 开始，读 4 个   → 错位了！
  行 2: ? ? ? ?    ← 从 12 开始           → 完全乱了！
```

**视觉效果**：图片会出现明显的**斜纹/倾斜**，像是被"剪切"变形了：

```
  正确 step：           错误 step：
  ┌──────────┐          ┌──────────┐
  │  🐱      │          │  🐱 ╱╱   │
  │  猫咪    │          │  ╱╱猫╱╱  │
  │  图片    │          │ ╱╱╱ 咪╱  │
  └──────────┘          └──────────┘
  正常显示               斜纹/错位
```

---

## 四、代码逐行详解

### 4.1 读取图片

```cpp
const cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_GRAYSCALE);
if (image.empty())
{
    // 显示错误信息
    return;
}
```

**关键点**：
- 总是检查 `image.empty()`！imread 失败不会抛异常，只会返回空 Mat
- `IMREAD_GRAYSCALE` 将图片转为灰度，方便演示 step 概念

### 4.2 正确的 QImage 转换

```cpp
const QImage qimage = matToQImage(image);
```

`matToQImage` 函数内部使用了正确的 `image.step`（自动从 Mat 获取）：

```cpp
QImage(image.data,         // 像素数据指针
       image.cols,         // 宽度
       image.rows,         // 高度
       image.step,         // ← 正确的行步长！
       QImage::Format_Grayscale8);
```

### 4.3 故意制造错误的 step

```cpp
const int wrongBytesPerLine = image.cols + 16;  // 比正确值多 16

QImage wrong(data,              // 同样的数据
             image.cols,        // 同样的宽度
             image.rows,        // 同样的高度
             wrongBytesPerLine,  // ← 错误的步长！
             QImage::Format_Grayscale8);
```

**效果**：QImage 以为每行有 `cols + 16` 个字节，但实际只有 `cols` 个。它会把下一行的前 16 个字节当作上一行的尾部——导致**每行都往右偏移 16 像素**，画面出现斜纹。

---

## 五、imread 的内部工作流程

```
cv::imread("cat.jpg", IMREAD_GRAYSCALE) 的完整流程：

  1. 打开文件
     └─▶ fopen("cat.jpg", "rb")

  2. 读取文件头（前几个字节）
     └─▶ 判断格式：JPEG？PNG？BMP？
         JPEG 文件头：FF D8 FF
         PNG 文件头：89 50 4E 47
         BMP 文件头：42 4D

  3. 调用对应的解码器
     └─▶ JPEG 解码器（libjpeg / libjpeg-turbo）
         ① DCT 反变换
         ② 色彩空间转换（YCbCr → BGR）
         ③ 上采样（如果有色度下采样）

  4. 根据 flags 做后处理
     └─▶ IMREAD_GRAYSCALE → BGR 转灰度
         Gray = 0.299R + 0.587G + 0.114B

  5. 创建 cv::Mat 并返回
     └─▶ Mat(rows, cols, CV_8UC1, data)
```

**文件头"魔数"快速识别表**：

| 格式 | 魔数（十六进制） | 魔数（ASCII） | 文件头字节数 |
|------|-----------------|--------------|------------|
| JPEG | `FF D8 FF` | 不可见 | 2~3 |
| PNG | `89 50 4E 47` | `‰PNG` | 8 |
| BMP | `42 4D` | `BM` | 2 |
| GIF | `47 49 46 38` | `GIF8` | 4 |
| TIFF (LE) | `49 49 2A 00` | `II*·` | 4 |
| TIFF (BE) | `4D 4D 00 2A` | `MM·*` | 4 |
| WebP | `52 49 46 46` | `RIFF` | 4 |

> OpenCV 读取文件时，先读这几个字节判断格式，再调用对应的解码器。

---

## 六、动手实验

### 实验 1：不同模式读取同一张图

```cpp
cv::Mat c = cv::imread("cat.jpg", cv::IMREAD_COLOR);
cv::Mat g = cv::imread("cat.jpg", cv::IMREAD_GRAYSCALE);
cv::Mat u = cv::imread("cat.jpg", cv::IMREAD_UNCHANGED);

std::cout << "COLOR:      " << c.cols << "x" << c.rows 
          << " channels=" << c.channels() << " step=" << c.step << std::endl;
std::cout << "GRAYSCALE:  " << g.cols << "x" << g.rows 
          << " channels=" << g.channels() << " step=" << g.step << std::endl;
std::cout << "UNCHANGED:  " << u.cols << "x" << u.rows 
          << " channels=" << u.channels() << " step=" << u.step << std::endl;
```

### 实验 2：测试不同的错误 step 值

```cpp
// 试试不同的偏移量，观察斜纹的角度变化
int offsets[] = {1, 4, 8, 16, 32, -8};
for (int off : offsets) {
    int wrongStep = image.cols + off;
    // 用 wrongStep 创建 QImage，观察效果...
}
// 偏移越大 → 斜纹越明显
// 负偏移 → 往另一个方向倾斜
```

---

## 七、常见问题

| 问题 | 原因 | 解决方法 |
|------|------|---------|
| imread 返回空 Mat | 路径错误 / 文件不存在 | 打印 `current_path()` 检查 |
| 图片颜色偏蓝 | BGR 和 RGB 搞混了 | OpenCV 默认 BGR，Qt 默认 RGB |
| 灰度图只有黑白蓝 | 把灰度 Mat 当 BGR 传给 QImage | 检查 channels() 是否匹配 |
| 图片有斜纹 | step 参数错误 | 使用 mat.step 而不是手工计算 |
| 图片显示比例不对 | resize 时宽高写反了 | OpenCV: (cols, rows)，注意顺序 |

---

## 八、术语表

| 术语 | 英文 | 含义 |
|------|------|------|
| 读取 | imread | OpenCV 读取图片文件到内存 |
| 解码 | Decode | 将压缩的文件数据还原为像素数组 |
| 行步长 | Step / Stride | 一行像素在内存中占的字节数 |
| 内存对齐 | Alignment | 数据地址按特定倍数排列以提升性能 |
| 填充 | Padding | 为对齐而在行末添加的无效字节 |
| 灰度 | Grayscale | 只有亮暗没有颜色的图像 |
| 文件头 | Header | 文件开头的标识字节，用于判断格式 |

---

## 九、知识地图

```
          你在这里
             ▼
  ① 生成并保存 → ② 读取并显示 → ③ 窗口交互
     imwrite       imread ★       namedWindow
     ↑              ↑
     生成的图片      本课读取它

  本课的核心知识点 step 会在后续所有 Mat↔QImage 转换中用到。
```

---

## 十、记忆口诀

```
🧠 imread 三模式：

  "彩灰原，321"
  ──────────────────────
  IMREAD_COLOR      → 3 通道（默认）
  IMREAD_GRAYSCALE  → 1 通道
  IMREAD_UNCHANGED  → 原始通道数

🧠 灰度公式：

  "红三绿六蓝一"
  Gray = 0.299R + 0.587G + 0.114B
         ~0.3     ~0.6     ~0.1
  绿色权重最大 → 人眼最敏感

🧠 Step 口诀：

  "Step 是一行的字节宽，对齐填充别忘掉"
  step ≥ cols × channels × sizeof(pixel)
  永远用 mat.step，别自己算
```

---

## 十一、新手雷区

```cpp
// ❌ 雷区 1：不检查 imread 结果
cv::Mat img = cv::imread("cat.jpg");
cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);  // 如果 img 为空 → 崩溃！

// ✅ 正确：永远先检查
cv::Mat img = cv::imread("cat.jpg");
if (img.empty()) {
    std::cerr << "读取失败: cat.jpg" << std::endl;
    return;
}
```

```cpp
// ❌ 雷区 2：自己算 step / bytesPerLine
QImage qimg(mat.data, mat.cols, mat.rows,
            mat.cols * mat.channels(),  // 手算步长 ← 可能错！
            QImage::Format_RGB888);

// ✅ 正确：用 mat.step
QImage qimg(mat.data, mat.cols, mat.rows,
            mat.step,  // ← 让 OpenCV 告诉你正确值
            QImage::Format_RGB888);
```

```cpp
// ❌ 雷区 3：忘记 BGR→RGB 转换就传给 Qt
QImage qimg(mat.data, mat.cols, mat.rows, mat.step,
            QImage::Format_RGB888);  // Qt 期望 RGB，但 mat 是 BGR！

// ✅ 正确：先转换
cv::Mat rgb;
cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
QImage qimg(rgb.data, rgb.cols, rgb.rows, rgb.step,
            QImage::Format_RGB888);
```

---

## 十二、思考题

1. **用 `IMREAD_COLOR` 读取一张灰度 PNG，得到的 Mat 有几个通道？**
   提示：IMREAD_COLOR 强制转换为 3 通道，即使原图是灰度的。

2. **为什么错误的 step 导致的斜纹角度和 step 偏差成正比？**
   提示：每行多读 N 个字节 = 画面每行向右偏移 N 像素。

3. **一张 1920×1080 的 BGR 图片在内存中至少占多少字节？**
   提示：1920 × 1080 × 3 = ?

---

## 十三、速查卡片

```
┌─────────────────── 课程 02 速查 ───────────────────┐
│                                                    │
│  读取图片:                                          │
│    cv::Mat img = cv::imread("path", flags);        │
│                                                    │
│  读取模式:                                          │
│    IMREAD_COLOR      → BGR 3通道（默认）            │
│    IMREAD_GRAYSCALE  → 灰度 1通道                   │
│    IMREAD_UNCHANGED  → 保留原始通道                  │
│                                                    │
│  必检项:                                            │
│    if (img.empty()) { /* 读取失败 */ }              │
│                                                    │
│  Step（行步长）:                                    │
│    mat.step = 一行占的字节数（含填充）               │
│    永远用 mat.step，不要自己算                       │
│                                                    │
│  Mat→QImage:                                       │
│    QImage(mat.data, cols, rows, mat.step, format)  │
│    记得 BGR→RGB 转换！                              │
│                                                    │
└────────────────────────────────────────────────────┘
```

---

## 十四、延伸阅读

- [cv::imread 官方文档](https://docs.opencv.org/4.x/d4/da8/group__imgcodecs.html#ga288b8b3da0892bd651fce07b3bbd3a56) — 所有支持的格式和 flags
- [Mat::step 详解](https://docs.opencv.org/4.x/d3/d63/classcv_1_1Mat.html#aa90cea495029c7d1ee0a41361ccecdf3) — 内存布局相关字段
- [图像文件读写教程](https://docs.opencv.org/4.x/d4/da8/group__imgcodecs.html) — imgcodecs 模块概览
