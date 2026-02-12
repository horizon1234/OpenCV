# 课程 01：生成并保存图片（cv::imwrite）

> 适合人群：零基础初学者 | 预计阅读时间：20 分钟

---

## 零、预备知识

在开始之前，你需要了解以下概念：

### 什么是"像素"？

**像素（Pixel）** 是图片的最小组成单位，就像马赛克瓷砖一样。一张 640×480 的图片，就是由 640 列 × 480 行 = **307,200 个小方块**组成的。

```
一张 8×6 像素的"图片"长这样（每个 ■ 是一个像素）：

  ■ ■ ■ ■ ■ ■ ■ ■    ← 第 0 行（共 8 个像素）
  ■ ■ ■ ■ ■ ■ ■ ■    ← 第 1 行
  ■ ■ ■ ■ ■ ■ ■ ■    ← 第 2 行
  ■ ■ ■ ■ ■ ■ ■ ■    ← 第 3 行
  ■ ■ ■ ■ ■ ■ ■ ■    ← 第 4 行
  ■ ■ ■ ■ ■ ■ ■ ■    ← 第 5 行

  总共 8 × 6 = 48 个像素
```

### 什么是"通道"？

每个像素可以有多个数值来描述它的颜色：

| 通道数 | 名称 | 含义 | 每个像素存什么 |
|--------|------|------|--------------|
| 1 | 灰度图 | 只有亮暗之分 | 1 个数：亮度（0=黑，255=白） |
| 3 | BGR 彩色图 | 蓝/绿/红三色混合 | 3 个数：B, G, R 各 0~255 |
| 4 | BGRA 图 | 彩色 + 透明度 | 4 个数：B, G, R, A（A=透明度） |

```
灰度像素：  [128]              → 中灰色
BGR 像素：  [255, 0, 0]        → 纯蓝色（OpenCV 用 BGR 不是 RGB！）
BGRA 像素： [255, 0, 0, 200]   → 蓝色 + 半透明
```

> **注意**：OpenCV 的颜色顺序是 **B-G-R**（蓝绿红），而不是日常说的 R-G-B（红绿蓝）。这是 OpenCV 的历史遗留设计，初学者很容易搞混。

### 什么是 cv::Mat？

`cv::Mat` 是 OpenCV 中**最核心的数据结构**，用于存储图片。你可以把它想象成一个**二维表格**（矩阵），每个格子里存着一个或多个数字：

```
一张 4×3 的灰度图在 cv::Mat 中的存储方式：

    列0   列1   列2   列3
  ┌─────┬─────┬─────┬─────┐
行0│ 0   │ 64  │ 128 │ 192 │
  ├─────┼─────┼─────┼─────┤
行1│ 32  │ 96  │ 160 │ 224 │
  ├─────┼─────┼─────┼─────┤
行2│ 64  │ 128 │ 192 │ 255 │
  └─────┴─────┴─────┴─────┘

  Mat.rows = 3（行数 = 图片高度）
  Mat.cols = 4（列数 = 图片宽度）
  Mat.channels() = 1（灰度图只有 1 个通道）
```

---

## 一、本课目标

本课你将学会：

1. **用代码生成一张图片**——不是从文件读取，而是逐像素"画"出来
2. **用 `cv::imwrite` 保存图片**到硬盘
3. 理解图片的**内存布局**——像素在内存中是怎么排列的

```
整体流程：

  ┌──────────────┐    ┌───────────────┐    ┌──────────────┐
  │ 创建空白 Mat  │───▶│ 逐像素填充颜色 │───▶│ imwrite 保存  │
  │ 640×480 BGRA │    │ 渐变色彩      │    │ 为 PNG 文件   │
  └──────────────┘    └───────────────┘    └──────────────┘
```

---

## 二、cv::imwrite 详解

### 2.1 函数签名

```cpp
bool cv::imwrite(const std::string& filename, 
                 const cv::Mat& img,
                 const std::vector<int>& params = std::vector<int>());
```

| 参数 | 含义 | 示例 |
|------|------|------|
| `filename` | 输出文件路径（含扩展名） | `"output.png"`, `"result.jpg"` |
| `img` | 要保存的图片（cv::Mat） | 你创建或处理后的图片 |
| `params` | 可选参数（压缩质量等） | 通常省略 |
| 返回值 | `true` 成功，`false` 失败 | — |

### 2.2 支持的图片格式

`imwrite` 通过**文件扩展名**自动判断要保存为什么格式：

| 扩展名 | 格式 | 特点 | 适合场景 |
|--------|------|------|---------|
| `.png` | PNG | 无损压缩，支持透明度 | 需要精确保留像素值、需要透明背景 |
| `.jpg` / `.jpeg` | JPEG | 有损压缩，文件小 | 照片、网页图片 |
| `.bmp` | BMP | 无压缩，文件大 | 简单场景、调试 |
| `.tiff` | TIFF | 无损，支持高位深 | 科研、医疗影像 |
| `.webp` | WebP | 有损/无损，文件很小 | 网页优化 |

```
格式选择决策树：

  需要透明度？──── 是 ──▶ .png
       │
      否
       │
  需要无损？──── 是 ──▶ .png 或 .tiff
       │
      否
       │
  文件大小重要？── 是 ──▶ .jpg（quality 调低）或 .webp
       │
      否
       │
  ──▶ .png（默认安全选择）
```

**同一张 640×480 BGRA 图片保存为不同格式的文件大小对比**：

```
  格式        文件大小     压缩比     原始数据 1,228,800 字节
  ────────────────────────────────────────────────────────
  .bmp       1,228 KB    ██████████████████████████ 1:1
  .png        ~150 KB    ███                        ~1:8
  .jpg(95)    ~100 KB    ██                         ~1:12
  .jpg(50)     ~25 KB    █                          ~1:49
  .webp        ~40 KB    █                          ~1:30

  注意：JPEG 保存 BGRA 图片时会自动丢弃 Alpha 通道
```

| 格式 | 有损/无损 | 支持透明 | 支持16位 | 典型压缩比 |
|------|----------|---------|---------|-----------|
| PNG | 无损 | ✅ Alpha | ✅ | 2~10× |
| JPEG | 有损 | ❌ | ❌ | 10~50× |
| BMP | 无压缩 | ❌ | ❌ | 1× |
| TIFF | 无损 | ✅ | ✅ 32位 | 1~3× |
| WebP | 可选 | ✅ Alpha | ❌ | 10~30× |

### 2.3 压缩参数

```cpp
// JPEG 质量控制（0~100，默认 95）
cv::imwrite("photo.jpg", img, {cv::IMWRITE_JPEG_QUALITY, 85});

// PNG 压缩级别（0~9，默认 3，越大文件越小但越慢）
cv::imwrite("image.png", img, {cv::IMWRITE_PNG_COMPRESSION, 6});
```

```
JPEG 质量 vs 文件大小（一张 640×480 照片的典型值）：

  质量    文件大小    画质
  ─────────────────────────
  100     ~200 KB    最好（几乎无损）
   95     ~100 KB    非常好（默认）
   85     ~60 KB     好（肉眼难辨）
   50     ~25 KB     一般（仔细看有瑕疵）
   10     ~8 KB      差（明显模糊和色块）

  ← 低质量                高质量 →
  ← 小文件                大文件 →
```

---

## 三、代码逐行详解

### 3.1 创建图片矩阵

```cpp
cv::Mat image(480, 640, CV_8UC4);
```

这一行创建了一个空白图片。拆开看：

```
cv::Mat image(480, 640, CV_8UC4);
              ───  ───  ────────
               │    │      │
               │    │      └── 类型代码：CV_8UC4
               │    │          CV  = OpenCV
               │    │          8U  = 8-bit unsigned（每个值 0~255）
               │    │          C4  = 4 channels（BGRA）
               │    │
               │    └── 640 列 = 图片宽度
               │
               └── 480 行 = 图片高度
```

常见的类型代码：

| 代码 | 含义 | 用途 |
|------|------|------|
| `CV_8UC1` | 8-bit 单通道 | 灰度图 |
| `CV_8UC3` | 8-bit 三通道 | BGR 彩色图 |
| `CV_8UC4` | 8-bit 四通道 | BGRA（含透明度） |
| `CV_32FC1` | 32-bit 浮点单通道 | 科学计算 |

### 3.2 逐像素填充颜色

```cpp
for (int y = 0; y < image.rows; ++y)
{
    for (int x = 0; x < image.cols; ++x)
    {
        cv::Vec4b &bgra = image.at<cv::Vec4b>(y, x);
        bgra[0] = static_cast<uchar>(255 * x / image.cols);   // B：从左到右渐变
        bgra[1] = static_cast<uchar>(255 * y / image.rows);   // G：从上到下渐变
        bgra[2] = static_cast<uchar>(255 - bgra[0]);          // R：与 B 反向
        bgra[3] = static_cast<uchar>(200);                    // A：半透明
    }
}
```

逐行解释：

```
image.at<cv::Vec4b>(y, x)
       ──────────  ─  ─
           │       │  │
           │       │  └── x = 列号（水平位置）
           │       └── y = 行号（垂直位置）
           └── Vec4b = 一个包含 4 个 uchar 值的向量（B, G, R, A）

返回的是引用（&bgra），所以修改 bgra 就是修改图片中那个像素
```

**颜色渐变逻辑**：

```
                x = 0          x = 320        x = 639
              （最左边）       （中间）        （最右边）
  ──────────────────────────────────────────────────────
  B 通道：    0（黑）     ──▶  128（中等）──▶  255（最亮）
  R 通道：    255（最亮）  ──▶  127（中等）──▶  0（黑）
  
  → 左边偏红，右边偏蓝（因为 R 和 B 反向）

                y = 0          y = 240        y = 479
              （最上面）       （中间）        （最下面）
  ──────────────────────────────────────────────────────
  G 通道：    0（黑）     ──▶  128（中等）──▶  255（最亮）
  
  → 上面 G 少，下面 G 多 → 下半部分偏绿

  整体效果：
  ┌─── 左上角：R 高 G 低 B 低 → 红色
  │    右上角：R 低 G 低 B 高 → 蓝色
  │    左下角：R 高 G 高 B 低 → 黄色
  │    右下角：R 低 G 高 B 高 → 青色
  └─── 中间：各色混合 → 渐变过渡
```

**四个角的精确 BGR 值参考表**：

| 位置 | B（蓝） | G（绿） | R（红） | A（透明度） | 颜色 |
|------|--------|---------|---------|------------|------|
| 左上 (0,0) | 0 | 0 | 255 | 200 | 🔴 红色 |
| 右上 (0,639) | 255 | 0 | 0 | 200 | 🔵 蓝色 |
| 左下 (479,0) | 0 | 255 | 255 | 200 | 🟡 黄色 |
| 右下 (479,639) | 255 | 255 | 0 | 200 | 🟢 青色 |
| 正中 (240,320) | 128 | 128 | 127 | 200 | 灰色 |

> 记住：**BGR** 顺序！`bgra[0]` 是蓝色不是红色。

### 3.3 保存图片

```cpp
if (!cv::imwrite(outputPath, image))
{
    // 保存失败
}
else
{
    // 保存成功
}
```

**常见失败原因**：

| 原因 | 解决方法 |
|------|---------|
| 路径不存在 | 确保目录已创建，或使用相对路径 |
| 没有写权限 | 检查文件夹权限 `ls -la` |
| 扩展名不支持 | 使用 `.png`, `.jpg`, `.bmp` 等标准格式 |
| 图片为空 | 检查 `image.empty()` |

---

## 四、cv::Mat 的内存布局

理解内存布局是深入学习 OpenCV 的关键。

### 4.1 连续存储

对于一张 4 列 × 3 行的 BGRA 图片，内存中是这样排列的：

```
mat.data 指向的内存（一维连续数组）：

行 0: [B G R A] [B G R A] [B G R A] [B G R A]   ← 4 个像素 × 4 字节 = 16 字节
行 1: [B G R A] [B G R A] [B G R A] [B G R A]   ← 16 字节
行 2: [B G R A] [B G R A] [B G R A] [B G R A]   ← 16 字节
      └──────── 像素 0 ────────┘                    总计 48 字节
      每个通道值占 1 字节（uchar = 0~255）
```

### 4.2 Step（步长）

`step` 是**一行数据在内存中占多少字节**。通常 `step = cols × channels × sizeof(每个值)`：

```
对于 640×480 CV_8UC4 的图片：

  step = 640 × 4 × 1 = 2560 字节/行

  要访问第 y 行第 x 列的像素：
  地址 = data + y × step + x × 4

  例如：访问 (100, 200) 位置的蓝色通道值：
  地址 = data + 100 × 2560 + 200 × 4 + 0
       = data + 256800
```

> **为什么需要 step？** 有时为了内存对齐（提高 CPU 读取速度），OpenCV 会在每行末尾加几个字节的填充（padding），导致 step > cols × channels。所以永远不要假设 step 等于行的像素数据大小，应该用 `mat.step` 获取。

### 4.3 at vs ptr 访问方式对比

```cpp
// 方法 1：at<T>(y, x) — 有边界检查（debug 模式下），方便但较慢
cv::Vec4b pixel = image.at<cv::Vec4b>(y, x);

// 方法 2：ptr<T>(y) — 获取一整行的指针，快速遍历
cv::Vec4b *row = image.ptr<cv::Vec4b>(y);
for (int x = 0; x < image.cols; ++x) {
    row[x][0] = 255; // B 通道
}

// 方法 3：直接操作 data 指针 — 最快但最危险
uchar *p = image.data + y * image.step + x * image.elemSize();
```

| 方法 | 速度 | 安全性 | 适合场景 |
|------|------|--------|---------|
| `at<T>` | 慢 | 高 | 调试、随机访问少量像素 |
| `ptr<T>` | 快 | 中 | 逐行遍历（推荐） |
| `data` 指针 | 最快 | 低 | 性能极致优化 |

**性能直观对比**（遍历 1920×1080 BGRA 图片的相对耗时）：

```
  方法           相对耗时      速度条
  ─────────────────────────────────────────
  at<Vec4b>     ~5.0 ms      ████████████████████  最慢
  ptr<Vec4b>    ~1.5 ms      ██████                快 3 倍
  data 指针      ~1.2 ms      █████                 最快
  
  建议：日常开发用 ptr，性能瓶颈用 data
```

---

## 五、完整代码

```cpp
cv::Mat ImwriteLessonWidget::generateImage()
{
    // 1. 创建 480 行 × 640 列的 BGRA 图片
    cv::Mat image(480, 640, CV_8UC4);
    
    // 2. 逐像素填充渐变色
    for (int y = 0; y < image.rows; ++y)
    {
        for (int x = 0; x < image.cols; ++x)
        {
            cv::Vec4b &bgra = image.at<cv::Vec4b>(y, x);
            bgra[0] = static_cast<uchar>(255 * x / image.cols);   // B 渐变
            bgra[1] = static_cast<uchar>(255 * y / image.rows);   // G 渐变
            bgra[2] = static_cast<uchar>(255 - bgra[0]);          // R 反向
            bgra[3] = static_cast<uchar>(200);                    // A 半透明
        }
    }
    return image;
}

void ImwriteLessonWidget::generateAndShowImage()
{
    const cv::Mat image = generateImage();
    const std::string outputPath = "generated_from_imwrite.png";

    // 3. 保存为 PNG 文件
    if (!cv::imwrite(outputPath, image))
    {
        statusLabel->setText("保存失败");
    }
    else
    {
        statusLabel->setText("已保存：generated_from_imwrite.png");
    }

    // 4. 转换为 QImage 在界面上显示
    const QImage qimage = matToQImage(image);
    imageLabel->setPixmap(QPixmap::fromImage(qimage));
}
```

---

## 六、动手实验

### 实验 1：修改颜色公式

尝试修改 `generateImage()` 中的颜色公式，观察效果变化：

```cpp
// 实验 a：纯红色渐变
bgra[0] = 0;
bgra[1] = 0;
bgra[2] = static_cast<uchar>(255 * x / image.cols);
bgra[3] = 255;

// 实验 b：棋盘格
bgra[0] = ((x / 32 + y / 32) % 2 == 0) ? 255 : 0;
bgra[1] = bgra[0];
bgra[2] = bgra[0];
bgra[3] = 255;

// 实验 c：同心圆
int cx = image.cols / 2, cy = image.rows / 2;
double dist = std::sqrt((x-cx)*(x-cx) + (y-cy)*(y-cy));
uchar v = static_cast<uchar>(128 + 127 * std::sin(dist * 0.05));
bgra[0] = bgra[1] = bgra[2] = v;
bgra[3] = 255;
```

### 实验 2：尝试不同格式

```cpp
cv::imwrite("test.png", image);   // PNG：文件大，无损
cv::imwrite("test.jpg", image);   // JPEG：文件小，有损
cv::imwrite("test.bmp", image);   // BMP：文件最大，无压缩

// 对比三个文件的大小
// ls -la test.*
```

---

## 七、常见问题

| 问题 | 原因 | 解决方法 |
|------|------|---------|
| 保存的图片颜色不对 | 混淆了 RGB 和 BGR | OpenCV 默认 BGR 顺序，调换 [0] 和 [2] |
| 保存的 JPEG 没有透明度 | JPEG 不支持 Alpha 通道 | 改用 PNG 格式 |
| `imwrite` 返回 false | 路径错误或无写权限 | 检查路径和权限 |
| 保存后文件为 0 字节 | Mat 为空 | 检查 `image.empty()` |
| `at<>` 崩溃 | 越界访问 | 确保 y < rows, x < cols |

---

## 八、术语表

| 术语 | 英文 | 含义 |
|------|------|------|
| 像素 | Pixel | 图片的最小单位 |
| 通道 | Channel | 描述像素颜色的数值个数 |
| 行步长 | Step / Stride | 一行像素在内存中占多少字节 |
| 无损压缩 | Lossless | 压缩后可完美还原原始数据 |
| 有损压缩 | Lossy | 压缩后有不可逆的质量损失 |
| Alpha | Alpha | 透明度通道，0=全透明，255=不透明 |
| Mat | Matrix | OpenCV 的图像/矩阵数据结构 |

---

## 九、知识地图

```
你在这里
   ▼
┌──────────────────────────────────────────────────────────┐
│ ① 生成并保存 → ② 读取并显示 → ③ 窗口交互              │
│    imwrite        imread         namedWindow             │
│    ★ 本课         ↑依赖本课                               │
│                                                          │
│ ④ 腐蚀膨胀 → ⑤ 边界提取                                 │
│                                                          │
│ ⑥ Gamma → ⑦ 直方图 → ⑧ 截断 → ⑨ 颜色 → ⑩ 反相         │
│                              → ⑪ 二值化 → ⑫ 对比度拉伸   │
└──────────────────────────────────────────────────────────┘

前置知识：无（本课是起点）
后续课程：课程 02（imread 读取本课生成的图片）
```

---

## 十、记忆口诀

```
🧠 cv::Mat 口诀：

  "行高列宽，类型看码"
  ──────────────────────
  Mat(rows, cols, type)
       │     │     │
      行数  列数   CV_8UC4 → 8位无符号4通道

🧠 imwrite 口诀：

  "写文件靠扩展名，成功返回布尔真"
  ──────────────────────────────
  bool ok = imwrite("name.png", mat);
                         │
                     扩展名决定格式

🧠 像素访问口诀：

  "at 安全慢，ptr 快一行，data 指针最疯狂"
```

---

## 十一、新手雷区

```cpp
// ❌ 雷区 1：RGB 和 BGR 搞混
cv::Vec4b pixel;
pixel[0] = 255;  // 新手以为这是 R（红色），实际是 B（蓝色）！
pixel[2] = 255;  // 新手以为这是 B（蓝色），实际是 R（红色）！

// ✅ 正确理解：OpenCV 的通道顺序永远是 B-G-R-A
pixel[0] = 255;  // B = 蓝色
pixel[1] = 0;    // G = 绿色
pixel[2] = 255;  // R = 红色
pixel[3] = 200;  // A = 透明度
```

```cpp
// ❌ 雷区 2：Mat 构造函数参数顺序搞反
cv::Mat image(640, 480, CV_8UC4);  // 640 行 × 480 列？
// 实际上：640 是 rows（高度），480 是 cols（宽度）
// 如果你想要宽 640 高 480 的图：

// ✅ 正确
cv::Mat image(480, 640, CV_8UC4);  // 480 行（高） × 640 列（宽）
```

```cpp
// ❌ 雷区 3：不检查 imwrite 返回值
cv::imwrite("output.png", image);  // 失败了也不知道！

// ✅ 正确
if (!cv::imwrite("output.png", image)) {
    std::cerr << "保存失败！" << std::endl;
}
```

---

## 十二、思考题

1. **为什么 OpenCV 选择 BGR 而不是 RGB？**
   提示：和早期的摄像机硬件设计有关，历史遗留至今。

2. **如果把一张 CV_8UC4 的 BGRA 图片保存为 `.jpg` 格式，会发生什么？**
   提示：JPEG 不支持 Alpha 通道，OpenCV 会怎么处理？

3. **`image.at<cv::Vec4b>(y, x)` 中为什么先 y 后 x？**
   提示：Mat 是矩阵，矩阵的下标惯例是 (行, 列)。

---

## 十三、速查卡片

```
┌─────────────────── 课程 01 速查 ───────────────────┐
│                                                    │
│  创建 Mat:                                          │
│    cv::Mat img(rows, cols, CV_8UC4);               │
│                                                    │
│  访问像素:                                          │
│    cv::Vec4b &px = img.at<cv::Vec4b>(y, x);        │
│    px[0]=B  px[1]=G  px[2]=R  px[3]=A              │
│                                                    │
│  保存图片:                                          │
│    cv::imwrite("file.png", img);                   │
│                                                    │
│  常用类型码:                                        │
│    CV_8UC1=灰度  CV_8UC3=BGR  CV_8UC4=BGRA         │
│                                                    │
│  属性:                                              │
│    img.rows  img.cols  img.channels()               │
│    img.step  img.data  img.empty()                  │
│                                                    │
└────────────────────────────────────────────────────┘
```

---

## 十四、延伸阅读

- [cv::Mat 官方文档](https://docs.opencv.org/4.x/d3/d63/classcv_1_1Mat.html) — Mat 类的完整 API
- [cv::imwrite 官方文档](https://docs.opencv.org/4.x/d4/da8/group__imgcodecs.html#gabbc7ef1aa2edfaa87772f1202d67e0ce) — 支持的格式和参数
- [OpenCV 像素访问教程](https://docs.opencv.org/4.x/db/da5/tutorial_how_to_scan_images.html) — 三种像素遍历方式性能对比
