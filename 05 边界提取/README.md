# 课程 05：腐蚀应用——边界提取

> 适合人群：零基础初学者 | 预计阅读时间：20 分钟

---

## 零、回顾上节课

上节课我们学了**腐蚀（Erode）**：让亮区域向内缩小一圈。这节课我们要用腐蚀来做一件很酷的事——**提取物体的边界轮廓**。

```
上节课学的：                    这节课要做的：
  原图 ──腐蚀──▶ 缩小的图        原图 - 腐蚀后的图 = 边界！
```

---

## 一、本课目标

本课你将学会：

1. 理解**边界提取的数学原理**——原图减去腐蚀图
2. 使用 `cv::absdiff` 计算**绝对差值**
3. 通过滑动条**交互控制边界粗细**

---

## 二、边界提取的原理

### 2.1 核心思想

**边界 = 原图 - 腐蚀后的图**

```
可以这样理解：

  原图（灰度）：
  ░░░░░░░░░░░░░░
  ░░░░████████░░      白色区域 = 物体
  ░░████████████░     黑色区域 = 背景
  ░░████████████░
  ░░░░████████░░
  ░░░░░░░░░░░░░░

  腐蚀后（缩小一圈）：
  ░░░░░░░░░░░░░░
  ░░░░░░████░░░░      物体向内缩了一圈
  ░░░░████████░░
  ░░░░████████░░
  ░░░░░░████░░░░
  ░░░░░░░░░░░░░░

  原图 - 腐蚀图 = ？
  ░░░░░░░░░░░░░░
  ░░░░████░░░░░░      消失的那一圈 = 边界！
  ░░██░░░░░░██░░
  ░░██░░░░░░██░░
  ░░░░████░░░░░░
  ░░░░░░░░░░░░░░

  原图中有、腐蚀图中没有的区域 → 只有边缘那一圈像素
```

### 2.2 为什么能提取边界？

```
详细推导（看一行像素的变化）：

  原图的一行像素值：
  位置:    0   1   2   3   4   5   6   7   8   9
  值:     0   0   0  255 255 255 255   0   0   0
          └─背景─┘  └──── 物体 ────┘  └─背景─┘

  腐蚀后（3×3核）：
  位置:    0   1   2   3   4   5   6   7   8   9
  值:     0   0   0   0  255 255   0   0   0   0
                     ↑               ↑
                   边缘被腐蚀掉了    这里也被腐蚀掉了

  相减（原图 - 腐蚀图）：
  位置:    0   1   2   3   4   5   6   7   8   9
  值:     0   0   0  255   0   0  255   0   0   0
                     └─┘             └─┘
                   左边界           右边界
                   
  只有边界位置有值 ✓
```

### 2.3 核大小 vs 边界粗细

```
核越大 → 腐蚀越多 → 原图和腐蚀图的差异越大 → 边界越粗

  核大小 = 3:           核大小 = 7:           核大小 = 15:
  ┌──────────┐          ┌──────────┐          ┌──────────┐
  │ ┌──────┐ │          │ ┌──────┐ │          │ ┌──────┐ │
  │ │      │ │          │ │░░░░░░│ │          │ │██████│ │
  │ │  细线 │ │          │ │░    ░│ │          │ │█    █│ │
  │ │      │ │          │ │░░░░░░│ │          │ │██████│ │
  │ └──────┘ │          │ └──────┘ │          │ └──────┘ │
  └──────────┘          └──────────┘          └──────────┘
  1 像素宽的边界         3 像素宽的边界         7 像素宽的边界
```

**核大小 → 边界宽度对照表**：

| erodeSize | 核大小 k | 腐蚀掉的层数 | 边界宽度（像素） | 适合场景 |
|----------|---------|------------|----------------|---------|
| 1 | 3×3 | 1 层 | 1 px | 细边界、小图 |
| 2 | 5×5 | 2 层 | 2 px | 一般用途 |
| 3 | 7×7 | 3 层 | 3 px | 带标注的可视化 |
| 5 | 11×11 | 5 层 | 5 px | 显眼的粗边界 |
| 10 | 21×21 | 10 层 | 10 px | 特殊效果 |

> **规律**：边界宽度 ≈ `erodeSize` 像素。核每大一级（+2），边界粗 1 像素。

---

## 三、代码逐行详解

### 3.1 灰度转换

```cpp
if (state.original.channels() == 3)
    cv::cvtColor(state.original, state.gray, cv::COLOR_BGR2GRAY);
else if (state.original.channels() == 4)
    cv::cvtColor(state.original, state.gray, cv::COLOR_BGRA2GRAY);
else
    state.gray = state.original.clone();
```

> 为什么先转灰度？边界提取通常在灰度图上做——彩色的三个通道分别做减法结果很混乱，不如灰度直观。

### 3.2 腐蚀

```cpp
const int k = state->erodeSize * 2 + 1;  // 保证奇数
cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(k, k));
cv::erode(state->gray, state->eroded, kernel);
```

### 3.3 计算差值（提取边界）

```cpp
cv::absdiff(state->gray, state->eroded, state->boundary);
```

**为什么用 `absdiff` 而不是直接减？**

```
直接减（src - dst）的问题：

  如果 src[i] = 50, dst[i] = 80：
    50 - 80 = -30  → uchar 存不了负数！
    → 溢出变成 226（错误值！）

absdiff 的做法：
    |50 - 80| = | -30 | = 30  → 正确 ✓

  所以 absdiff 更安全，永远返回非负值
```

$$
\text{boundary}(x,y) = | \text{gray}(x,y) - \text{eroded}(x,y) |
$$

### 3.4 滑动条控制边界粗细

```cpp
cv::createTrackbar("Erode", windowName, &state.erodeSize, 10, onErodeTrackbar, &state);

void onErodeTrackbar(int value, void *userdata)
{
    auto *state = static_cast<BoundaryState *>(userdata);
    state->erodeSize = std::max(1, value);  // 至少为 1，否则无效果
    updateBoundary(state);
}
```

---

## 四、与其他边缘检测方法的对比

形态学边界提取只是边缘检测的方法之一：

| 方法 | 原理 | 优点 | 缺点 |
|------|------|------|------|
| **形态学（本课）** | 原图 - 腐蚀图 | 简单直观，可控粗细 | 对噪声敏感 |
| Sobel | 一阶导数（梯度） | 可区分水平/垂直边缘 | 需选方向 |
| Canny | 多步骤（高斯模糊+梯度+非极大值抑制+双阈值） | 效果最好，细且连续 | 参数多，较复杂 |
| Laplacian | 二阶导数 | 各方向均匀 | 对噪声更敏感 |

**如何选择边缘检测方法？（决策流程）**：

```
  需要边缘检测 ─ 初学/快速原型？──── 是 ──▶ 形态学（本课）
            │
           否
            │
       需要方向信息？──── 是 ──▶ Sobel（可分水平/垂直）
            │
           否
            │
       需要最精确的细边？── 是 ──▶ Canny（工业标准）
            │
           否
            ├──▶ Laplacian（各方向均匀但噪声敏感）
```

| 对比维度 | 形态学边界 | Sobel | Canny | Laplacian |
|---------|-----------|-------|-------|-----------|
| 代码复杂度 | ⭐ 最简单 | ⭐⭐ | ⭐⭐⭐ | ⭐⭐ |
| 边界粗细 | 可调（核大小控制） | 固定 1~2px | 细 1px | 1~2px |
| 方向性 | 无 | 可分 X/Y | 无（全方向） | 无 |
| 抗噪能力 | 弱 | 中 | 强（自带高斯） | 弱 |
| 边界连续性 | 好 | 一般 | 最好（双阈值） | 一般 |
| 需要函数 | erode + absdiff | Sobel | Canny | Laplacian |

```
各方法的边缘效果直观对比：

  形态学边界：            Sobel 边缘：           Canny 边缘：
  ┌──────────┐          ┌──────────┐          ┌──────────┐
  │ ██████   │          │ ░█████░  │          │  █████   │
  │ █    █   │          │ █░░░░█░  │          │ █    █   │
  │ █    █   │          │ █░░░░█░  │          │ █    █   │
  │ ██████   │          │ ░█████░  │          │  █████   │
  └──────────┘          └──────────┘          └──────────┘
  粗，可控                有方向性              最细最精确
```

> **建议**：形态学边界提取最适合入门理解"什么是边缘"。学完本课后，可以逐步学习 Sobel 和 Canny 等更高级的方法。

---

## 五、morphologyEx——更多形态学操作

OpenCV 提供了一个统一的形态学函数，内部封装了各种组合操作：

```cpp
cv::morphologyEx(src, dst, operation, kernel);
```

| 操作 | 常量 | 公式 | 效果 |
|------|------|------|------|
| 腐蚀 | `MORPH_ERODE` | $\min$ | 缩小亮区 |
| 膨胀 | `MORPH_DILATE` | $\max$ | 扩大亮区 |
| 开运算 | `MORPH_OPEN` | 先腐蚀后膨胀 | 去小亮点 |
| 闭运算 | `MORPH_CLOSE` | 先膨胀后腐蚀 | 填小暗洞 |
| **梯度** | `MORPH_GRADIENT` | 膨胀 - 腐蚀 | **提取边界**（和本课方法类似！） |
| 顶帽 | `MORPH_TOPHAT` | 原图 - 开运算 | 提取亮细节 |
| 黑帽 | `MORPH_BLACKHAT` | 闭运算 - 原图 | 提取暗细节 |

**注意**：`MORPH_GRADIENT = dilate - erode`（膨胀减去腐蚀），而本课的方法是 `原图 - erode`。两者类似但会有细微差别——MORPH_GRADIENT 的边界比本课的方法略粗（约两倍）。

**morphologyEx 操作间的数学关系图**：

```
  基础操作：
             erode ──────────────────── dilate
               │                          │
               ▼                          ▼
  组合操作：  open = erode → dilate      close = dilate → erode
               │                          │
               ▼                          ▼
  差值操作：  tophat = src - open        blackhat = close - src
                        │
                        ▼
             gradient = dilate - erode（形态学梯度）
             boundary = src - erode    （本课方法）
```

| 操作 | 公式 | 结果图特征 |
|------|------|-----------|
| `src - erode` | 本课方法 | 内边界，宽 ≈ erodeSize |
| `dilate - src` | 外边界 | 外边界，宽 ≈ dilateSize |
| `dilate - erode` | 形态学梯度 | 内+外边界，宽 ≈ 2×size |

---

## 六、动手实验

### 实验 1：对比两种边界提取方法

```cpp
// 方法 A：原图 - 腐蚀（本课的方法）
cv::Mat eroded, boundaryA;
cv::erode(gray, eroded, kernel);
cv::absdiff(gray, eroded, boundaryA);

// 方法 B：morphologyEx 形态学梯度
cv::Mat boundaryB;
cv::morphologyEx(gray, boundaryB, cv::MORPH_GRADIENT, kernel);

// 对比两种结果
cv::imshow("Method A: src - erode", boundaryA);
cv::imshow("Method B: dilate - erode", boundaryB);
```

### 实验 2：彩色边界提取

```cpp
// 在每个通道分别做边界提取，然后合并
std::vector<cv::Mat> channels;
cv::split(color, channels);
for (auto &ch : channels) {
    cv::Mat eroded;
    cv::erode(ch, eroded, kernel);
    cv::absdiff(ch, eroded, ch);
}
cv::Mat colorBoundary;
cv::merge(channels, colorBoundary);
cv::imshow("Color Boundary", colorBoundary);
```

---

## 七、常见问题

| 问题 | 原因 | 解决方法 |
|------|------|---------|
| 边界图全黑 | 核太小（k=1 无效果） | 确保 erodeSize ≥ 1（k ≥ 3） |
| 边界太粗看不清细节 | 核太大 | 减小 erodeSize |
| 噪声很多 | 原图有噪声，边界提取会放大噪声 | 先高斯模糊再提取 |
| 只有部分边界 | 物体和背景对比度不够 | 先做对比度拉伸（课程 12） |

---

## 八、术语表

| 术语 | 英文 | 含义 |
|------|------|------|
| 边界提取 | Boundary Extraction | 从图像中提取物体的轮廓线 |
| 绝对差值 | Absolute Difference | 两个值之差的绝对值 |
| 形态学梯度 | Morphological Gradient | 膨胀图减去腐蚀图 |
| 开运算 | Opening | 先腐蚀后膨胀，去噪点 |
| 闭运算 | Closing | 先膨胀后腐蚀，填空洞 |
| 顶帽变换 | Top-Hat | 原图 - 开运算，提取亮细节 |
| 黑帽变换 | Black-Hat | 闭运算 - 原图，提取暗细节 |

---

## 九、知识地图

```
  ③ 窗口交互 → ④ 腐蚀与膨胀 → ⑤ 边界提取
                  ↑ 前置知识       ★ 本课
                  erode           absdiff(原图, 腐蚀图)

  本课是形态学的"毕业作品" → 用腐蚀实现了边缘检测。
  后续课程 06~12 转向"点运算"系列（逐像素处理）。

  边缘检测的进阶路线：
  形态学边界（本课） → Sobel 梯度 → Canny 边缘 → 轮廓检测
```

---

## 十、记忆口诀

```
🧠 边界提取核心：

  "原减腐，边界出"
  boundary = original - eroded

🧠 absdiff 口诀：

  "绝对差值不溢出，两图做减用它准"
  cv::absdiff(a, b, diff)  →  diff = |a - b|

🧠 核大小与边界粗细：

  "核越大，边越粗；核越小，边越细"
  k=3 → 1像素边界   k=7 → 3像素边界
```

---

## 十一、新手雷区

```cpp
// ❌ 雷区 1：用减法代替 absdiff
cv::Mat boundary = gray - eroded;
// 如果 eroded 某像素比 gray 大（浮点误差等），会溢出变成大值！

// ✅ 正确
cv::absdiff(gray, eroded, boundary);  // 绝对值，永远安全
```

```cpp
// ❌ 雷区 2：erodeSize 设为 0
int k = 0 * 2 + 1;  // k = 1 → 1×1 的核
cv::erode(gray, eroded, kernel);
// 1×1 核的腐蚀 = 无操作，灰度不变，边界全是 0！

// ✅ 正确：确保 erodeSize ≥ 1
state->erodeSize = std::max(1, value);
```

```cpp
// ❌ 雷区 3：直接对彩色图做 absdiff
cv::absdiff(color, erodedColor, boundary);
// 三通道分别做差，结果是"彩色边界"，不直观

// ✅ 推荐：先转灰度再提取边界
cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);
cv::erode(gray, eroded, kernel);
cv::absdiff(gray, eroded, boundary);
```

---

## 十二、思考题

1. **`原图 - 腐蚀图` 和 `膨胀图 - 原图` 提取的边界有什么区别？**
   提示：一个是"内边界"，一个是"外边界"。

2. **MORPH_GRADIENT（膨胀-腐蚀）的边界比本课方法粗多少？**
   提示：大约两倍——同时包含了内外两侧。

3. **如果图片有很多噪声，边界提取结果会怎样？怎么改善？**
   提示：噪声本身也有"灰度变化"，会被当作"边界"。

---

## 十三、速查卡片

```
┌─────────────────── 课程 05 速查 ───────────────────┐
│                                                    │
│  边界提取三步：                                     │
│    1. cv::erode(gray, eroded, kernel);              │
│    2. cv::absdiff(gray, eroded, boundary);          │
│    3. cv::imshow("边界", boundary);                 │
│                                                    │
│  等效方法（形态学梯度）：                            │
│    cv::morphologyEx(gray, bnd, MORPH_GRADIENT, k);  │
│    注意：梯度 = dilate - erode，边界约两倍粗        │
│                                                    │
│  核大小 → 边界粗细：                                │
│    k=3 → 细边界    k=7 → 中等    k=15 → 粗边界     │
│                                                    │
│  降噪建议：                                        │
│    cv::GaussianBlur(gray, blurred, Size(5,5), 0);  │
│    先模糊再提取边界                                  │
│                                                    │
└────────────────────────────────────────────────────┘
```

---

## 十四、延伸阅读

- [cv::absdiff 文档](https://docs.opencv.org/4.x/d2/de8/group__core__array.html#ga6fef31bc8c4071cbc114a758a2b79c14) — 绝对差值函数
- [Canny 边缘检测教程](https://docs.opencv.org/4.x/da/d22/tutorial_py_canny.html) — 更高级的边缘检测
- [形态学操作教程](https://docs.opencv.org/4.x/d9/d61/tutorial_py_morphological_ops.html) — 梯度、顶帽、黑帽详解
