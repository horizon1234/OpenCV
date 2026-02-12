# 课程 12：点运算——对比度拉伸（Contrast Stretching）

> 适合人群：零基础初学者 | 预计阅读时间：25 分钟

---

## 一、本课目标

本课你将学会：

1. 理解**对比度**的含义——图像灰度范围的宽窄
2. 理解**对比度拉伸**——把窄范围线性映射到 [0, 255]
3. 使用 `cv::minMaxLoc` + `cv::Mat::convertTo` 实现拉伸
4. 区分对比度拉伸与直方图均衡化的异同

---

## 二、什么是对比度？

### 2.1 直观理解

**对比度 = 图像中最亮和最暗之间的差距。**

```
类比——音量旋钮：

  低对比度（音量小）：       高对比度（音量大）：
  所有声音都差不多大           有很轻柔也有很响亮
  听起来"闷闷的"             听起来"有层次"

  ┌── 低对比度图像 ──┐      ┌── 高对比度图像 ──┐
  │ ▒▒▒▒▒▒▒▒▒▒▒▒▒▒ │      │ ░░░░▒▒▒▓▓▓████ │
  │ 灰度范围很窄      │      │ 灰度范围铺满      │
  │ [80, 180]        │      │ [0, 255]          │
  │ 看起来灰蒙蒙      │      │ 看起来清晰有力     │
  └──────────────────┘      └──────────────────┘
```

### 2.2 用直方图看对比度

```
低对比度：                         高对比度：
频率                               频率
↑                                  ↑
│        ╱╲                        │  ╱╲        ╱╲
│       ╱  ╲                       │ ╱  ╲  ╱╲ ╱  ╲
│      ╱    ╲                      │╱    ╲╱  ╲    ╲
│─────╱      ╲─────                │                ╲───
└──────────────────▶ 灰度          └──────────────────▶ 灰度
0    80    180   255               0               255
     ↑      ↑
  集中在一小段                      分布在整个范围
```

### 2.3 对比度拉伸的本质

```
对比度拉伸 = 把"窄"的灰度范围"拉"到 [0, 255]

  拉伸前：                    拉伸后：
  0          80   180     255     0                        255
  |──────────[█████]──────|       |[████████████████████████]|
              ↑   ↑                ↑                        ↑
           最暗  最亮             最暗→0                  最亮→255
           范围=100               范围=255

  80 → 0           (最暗变成 0)
  180 → 255         (最亮变成 255)
  130 → 127.5       (中间值等比例映射)

  所有灰度值都被"拉伸"了，间距变大了
```

---

## 三、数学公式

### 3.1 线性映射公式

$$
\text{dst}(x,y) = \frac{\text{src}(x,y) - \text{min}}{\text{max} - \text{min}} \times 255
$$

其中：
- $\text{min}$ = 图像中的最小灰度值
- $\text{max}$ = 图像中的最大灰度值

```
公式推导过程：

  目标：把 [min, max] 映射到 [0, 255]

  步骤 1：平移，让最小值变 0
    src - min  →  范围变为 [0, max-min]

  步骤 2：归一化到 [0, 1]
    (src - min) / (max - min)  →  范围 [0, 1]

  步骤 3：缩放到 [0, 255]
    (src - min) / (max - min) × 255  →  范围 [0, 255]

  合并成一步：
    dst = (src - min) × 255 / (max - min)
        = src × [255/(max-min)] + [-min×255/(max-min)]
        = src × alpha + beta

  其中：
    alpha = 255 / (max - min)     ← 缩放因子
    beta  = -min × 255 / (max - min)  ← 偏移量
```

### 3.2 映射曲线

```
  输出值
  255│              ╱
     │            ╱
     │          ╱
  128│        ╱        ← 斜率 = 255 / (max - min)
     │      ╱             比恒等变换（斜率=1）更陡
     │    ╱
     │  ╱
   0 ┼╱───────────────▶ 输入值
     min            max

  对比恒等变换（不拉伸）的曲线：

  恒等:               拉伸:
  255│     ╱           255│        ╱
     │   ╱                │      ╱
     │ ╱                  │    ╱
   0 │╱                 0 ┼──╱
     └──255              min  max

  拉伸的斜率更大 → 对比度更高
```

### 3.3 一个数值例子

```
假设图像最暗像素 min=60，最亮像素 max=200

  alpha = 255 / (200 - 60) = 255 / 140 ≈ 1.821
  beta  = -60 × 255 / 140 = -109.3

  各像素的变换：
  ┌────────┬──────────────────────┬────────┐
  │ 输入    │ 计算过程              │ 输出    │
  ├────────┼──────────────────────┼────────┤
  │  60    │ (60-60)×255/140     │   0    │ ← 最暗 → 0
  │  80    │ (80-60)×255/140     │  36    │
  │ 100    │ (100-60)×255/140    │  73    │
  │ 130    │ (130-60)×255/140    │ 127    │ ← 中间值 → 接近 128
  │ 160    │ (160-60)×255/140    │ 182    │
  │ 180    │ (180-60)×255/140    │ 218    │
  │ 200    │ (200-60)×255/140    │ 255    │ ← 最亮 → 255
  └────────┴──────────────────────┴────────┘

  输入范围 [60, 200] → 输出范围 [0, 255] ✅
```

---

## 四、核心 API

### 4.1 cv::minMaxLoc

```cpp
double minVal, maxVal;
cv::minMaxLoc(gray, &minVal, &maxVal);
// minVal = 图像中的最小灰度值
// maxVal = 图像中的最大灰度值
```

```
minMaxLoc 的完整签名：

  void cv::minMaxLoc(
      const cv::Mat& src,
      double* minVal,        // 最小值
      double* maxVal,        // 最大值
      cv::Point* minLoc,     // 最小值位置（可选，传 nullptr）
      cv::Point* maxLoc,     // 最大值位置（可选，传 nullptr）
      const cv::Mat& mask    // 掩膜（可选）
  );

  本课只需要 minVal 和 maxVal
```

### 4.2 cv::Mat::convertTo

```cpp
double alpha = 255.0 / (maxVal - minVal);
double beta  = -minVal * alpha;

gray.convertTo(stretched, CV_8U, alpha, beta);
// dst(x,y) = saturate_cast<uchar>( src(x,y) * alpha + beta )
```

```
convertTo 做的事等价于：

  对每个像素 p：
    1. 乘以 alpha:   p × 1.821 ≈ 放大
    2. 加上 beta:    + (-109.3) ≈ 平移
    3. 饱和截断:      限制在 [0, 255]

  ┌──────────────────────────────────────┐
  │ 为什么用 convertTo 而不是手动循环？   │
  │                                      │
  │ ① convertTo 内部用 SIMD 优化，极快   │
  │ ② 自动做 saturate_cast，不会溢出     │
  │ ③ 一行代码搞定                       │
  └──────────────────────────────────────┘
```

---

## 五、完整代码流程

```
┌──────────────────────────────────────────────┐
│  1. 读取原图                                  │
│     cv::imread("cat.jpg")                    │
│                                              │
│  2. 转灰度                                    │
│     cv::cvtColor(src, gray, COLOR_BGR2GRAY)  │
│                                              │
│  3. 找最值                                    │
│     cv::minMaxLoc(gray, &minVal, &maxVal)    │
│     例如：minVal=60, maxVal=200              │
│                                              │
│  4. 计算参数                                  │
│     alpha = 255 / (maxVal - minVal)          │
│     beta  = -minVal * alpha                  │
│                                              │
│  5. 线性变换                                  │
│     gray.convertTo(stretched, CV_8U, α, β)   │
│                                              │
│  6. 显示                                      │
│     imshow("原图", gray)                      │
│     imshow("拉伸", stretched)                 │
└──────────────────────────────────────────────┘
```

### 5.1 边界情况

```
如果 minVal == maxVal（整张图只有一种灰度）：

  alpha = 255 / (maxVal - minVal) = 255 / 0 = ∞  ← 除零错误！

  解决方案：
  if (maxVal - minVal < 1e-6) {
      gray.copyTo(stretched);  // 不做变换
  } else {
      double alpha = 255.0 / (maxVal - minVal);
      double beta = -minVal * alpha;
      gray.convertTo(stretched, CV_8U, alpha, beta);
  }
```

---

## 六、对比度拉伸 vs 直方图均衡化

```
二者都是"增强对比度"，但方法不同：

对比度拉伸（线性）：         直方图均衡化（非线性）：
┌─────────────────┐        ┌─────────────────┐
│ 输入 [80,180]    │        │ 输入（任意分布）  │
│      ↓ 线性映射   │        │      ↓ CDF 映射   │
│ 输出 [0, 255]    │        │ 输出（均匀分布）  │
└─────────────────┘        └─────────────────┘

拉伸：                     均衡化：
  ╱  （直线）                ╱─── （S 形曲线）
 ╱                          ╱
╱                         ╱╱
简单线性                   非线性，更强
```

| 特性 | 对比度拉伸 | 直方图均衡化（课程07） |
|------|----------|---------------------|
| 映射方式 | **线性** | **非线性**（CDF） |
| 可控性 | 高（只依赖 min/max） | 低（全自动） |
| 增强强度 | 温和 | 强烈 |
| 保持灰度顺序 | ✅ 是 | ✅ 是 |
| 保持灰度间距 | ✅ 等比例 | ❌ 不等比例 |
| 适用场景 | 整体偏暗/偏亮 | 直方图集中 |
| 可能的问题 | 对异常值敏感 | 可能过度增强 |

#### 增强方法选择决策表

| 你的图像问题 | 推荐方法 | 理由 |
|:----------:|:-------:|:----:|
| 整体偏暗或偏亮 | **对比度拉伸**（本课） | 线性映射，最简单有效 |
| 灰度集中在某区间 | 直方图均衡化（课程07） | CDF 展开，效果更强 |
| 局部对比度低 | CLAHE（课程07 延伸） | 分块自适应，保留细节 |
| 高光过曝需抑制 | 截断（课程08） | 削平亮部 |
| 需要精细的亮度控制 | Gamma 校正（课程06） | 非线性，可调参数 |
| 有少量异常像素干扰 | 百分位截断拉伸 | 忽略极端值 |

```
同一张低对比度图片的处理效果：

  原图直方图：        拉伸后：             均衡化后：
     ╱╲                   ╱╲              ╱╲  ╱╲  ╱╲
    ╱  ╲             ╱╲  ╱  ╲  ╱╲       ╱  ╲╱  ╲╱  ╲
  ──╱────╲──       ─╱──╲╱────╲╱──╲─    ╱────╲────╲────╲
  0  80 180 255    0            255     0            255
   集中在一段         拉开了             完全散开了
```

---

## 七、极端值的影响

### 7.1 问题：异常像素

```
如果图像 99% 的像素在 [80, 200]，但有几个异常像素：
  最小值 = 2（一个死像素）
  最大值 = 253（一个热像素）

  拉伸范围变成 [2, 253]，alpha = 255/251 ≈ 1.016
  几乎没有拉伸效果！

  ┌── 异常值影响 ──────────────────────────────┐
  │  正常范围 [80, 200]  → alpha=2.55, 效果好   │
  │  含异常 [2, 253]     → alpha=1.02, 没效果   │
  └────────────────────────────────────────────┘
```

### 7.2 解决方案：百分位截断

```cpp
// 不用绝对最小/最大，而是用 1% 和 99% 分位数
// 需要先计算直方图

int histSize = 256;
float range[] = {0, 256};
const float* histRange = {range};
cv::Mat hist;
cv::calcHist(&gray, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

int totalPixels = gray.rows * gray.cols;
int lowCut = totalPixels * 0.01;   // 1% 分位
int highCut = totalPixels * 0.99;  // 99% 分位

int cumulative = 0;
int minP = 0, maxP = 255;
for (int i = 0; i < 256; i++) {
    cumulative += hist.at<float>(i);
    if (cumulative >= lowCut && minP == 0) minP = i;
    if (cumulative >= highCut) { maxP = i; break; }
}

// 用 minP 和 maxP 代替 minVal 和 maxVal
double alpha = 255.0 / (maxP - minP);
double beta = -minP * alpha;
gray.convertTo(stretched, CV_8U, alpha, beta);
```

```
百分位截断示意图：

  频率
  ↑
  │    ╱╲
  │   ╱  ╲
  │  ╱    ╲
  │ ╱      ╲
  │╱        ╲
  └─┬───────┬─▶ 灰度值
    1%     99%
    ↑       ↑
   minP   maxP    ← 忽略两端 1% 的异常值

  比用绝对 min/max 更鲁棒
```

#### 极端值处理策略对比

| 策略 | 方法 | alpha 效果 | 鲁棒性 | 适用场景 |
|:----:|:----:|:---------:|:-----:|:-------:|
| 绝对最值 | `minMaxLoc` | 可能≈1（无效） | ❌ 差 | 无噪声/异常的理想图像 |
| 1%~99% 分位 | 直方图计算 | 合理放大 | ✅ 好 | **通用推荐** |
| 5%~95% 分位 | 直方图计算 | 放大更多 | ✅ 很好 | 噪声较多的图像 |
| 手动指定 | 固定 min/max | 可控 | ⭐ 最好 | 已知场景、批量处理 |

---

## 八、多通道对比度拉伸

```
如何对彩色图像做对比度拉伸？

  方法 1：分别对 B、G、R 三通道独立拉伸
    ● 简单直接
    ● 可能改变色彩平衡（因为三通道缩放不同）

  方法 2：转 HSV / YCrCb，只拉伸亮度通道
    ● 保持色彩平衡
    ● 更自然（推荐）

  方法 2 的流程：
  BGR → YCrCb → 只拉伸 Y 通道 → YCrCb → BGR
                     ↑
               对比度拉伸只在亮度上做
               色度通道(Cr, Cb)不动
```

#### 彩色图像对比度拉伸方法对比

| 维度 | BGR 三通道独立拉伸 | YCrCb 仅拉伸 Y 通道 |
|:----:|:----------------:|:------------------:|
| **实现难度** | ⭐ 简单（split→各自 convertTo→merge） | ⭐⭐ 中等（需色彩空间转换） |
| **色彩保真** | ❌ 可能偏色（三通道缩放因子不同） | ✅ 色彩平衡不变 |
| **对比度效果** | ✅ 强（各通道独立优化） | ✅ 好（仅亮度增强） |
| **适用场景** | 灰度图、不关心颜色 | 彩色图、需保持原色 |
| **代码行数** | ~8 行 | ~10 行 |
| **推荐度** | 灰度图首选 | **彩色图首选** |

---

## 九、动手实验

### 实验 1：人工创建低对比度图像

```cpp
cv::Mat low_contrast;
gray.convertTo(low_contrast, CV_8U, 0.3, 100);
// 缩小范围：[0,255] → [100, 176]

// 再拉伸回去
double mi, ma;
cv::minMaxLoc(low_contrast, &mi, &ma);
double a = 255.0 / (ma - mi);
double b = -mi * a;
cv::Mat restored;
low_contrast.convertTo(restored, CV_8U, a, b);

cv::imshow("原图", gray);
cv::imshow("低对比度", low_contrast);
cv::imshow("拉伸恢复", restored);
// 观察：拉伸后和原图非常接近！
```

### 实验 2：对比直方图均衡化

```cpp
cv::Mat stretched, equalized;

// 对比度拉伸
double mi, ma;
cv::minMaxLoc(gray, &mi, &ma);
gray.convertTo(stretched, CV_8U, 255.0/(ma-mi), -mi*255.0/(ma-mi));

// 直方图均衡化
cv::equalizeHist(gray, equalized);

cv::imshow("原图", gray);
cv::imshow("对比度拉伸", stretched);
cv::imshow("直方图均衡化", equalized);
// 对比两种方法的效果差异
```

### 实验 3：拉伸前后的直方图

```cpp
// 计算并显示拉伸前后的直方图
// 观察：拉伸后直方图应该铺满 [0, 255]
// 但间距变大了（因为是线性拉伸）
```

---

## 十、常见问题

| 问题 | 原因 | 解决方法 |
|------|------|---------|
| 拉伸后效果不明显 | 原图 min≈0, max≈255，已经是全范围 | 无需拉伸 |
| 拉伸后出现噪点 | 噪声也被放大了 | 先模糊降噪再拉伸 |
| 拉伸后颜色偏了 | 三通道独立拉伸导致色彩漂移 | 转 YCrCb 只拉伸 Y |
| 除以零崩溃 | max == min（单色图） | 代码中加判断 |
| 异常像素干扰 | 死/热像素导致 min/max 不准 | 用百分位截断 |

---

## 十一、各种增强方法总结

```
本系列课程的点运算方法一览：

  ┌────────────┬────────────┬───────────────────────┐
  │ 课程        │ 方法        │ 核心公式               │
  ├────────────┼────────────┼───────────────────────┤
  │ 06 灰度变换 │ Gamma 校正  │ dst = 255×(src/255)^γ │
  │ 07 直方图   │ 均衡化      │ dst = CDF(src)×255    │
  │ 08 截断     │ 阈值截断    │ dst = min(src, T)     │
  │ 09 颜色调整 │ 通道缩放    │ dst = src × scale     │
  │ 10 反相     │ 取反        │ dst = 255 - src       │
  │ 11 二值化   │ 阈值判断    │ dst = src>T ? 255 : 0 │
  │ 12 对比度   │ 线性拉伸    │ dst = (src-min)×α+β   │
  └────────────┴────────────┴───────────────────────┘

  它们的映射曲线对比：

  Gamma:    均衡化:    截断:     反相:     二值化:   拉伸:
  │  ╱      │ ╱──     T│────    │╲        │ ┌──    │   ╱
  │╱╱       │╱         │╱       │ ╲       │ │      │  ╱
  │╱        ╱          │        │  ╲      │─┘      │ ╱
  └──       └──        └──T     └──╲      └─T      └╱
```

---

## 十二、术语表

| 术语 | 英文 | 含义 |
|------|------|------|
| 对比度 | Contrast | 图像中最亮和最暗之间的差距 |
| 对比度拉伸 | Contrast Stretching | 将灰度范围线性映射到 [0,255] |
| 线性映射 | Linear Mapping | 用 y=ax+b 的直线做灰度变换 |
| 最小值 | Minimum | 图像中最暗像素的灰度值 |
| 最大值 | Maximum | 图像中最亮像素的灰度值 |
| 百分位 | Percentile | 排序后第 N% 位置的值 |
| 归一化 | Normalization | 将数据缩放到标准范围 |
| 缩放因子 | Scale Factor (alpha) | 线性变换中的乘法系数 |
| 偏移量 | Offset (beta) | 线性变换中的加法系数 |
| 饱和截断 | Saturate Cast | 超出范围的值被限制在范围内 |

---

## 十三、知识地图

```
  ⑨ 颜色调整 → ⑩ 反相 → ⑪ 二值化 → ⑫ 对比度拉伸
                                      ★ 本课（最后一课！）

  本课是"点运算"系列的终章。回顾整个系列：

  课程 01-05：图像 I/O + 窗口 + 形态学基础
  课程 06-12：七种点运算（逐像素变换）

  对比度拉伸 vs 直方图均衡化：
  ├── 均衡化（课程 07）：非线性，按 CDF 映射
  └── 拉伸（本课）   ：线性，按 min-max 映射

  convertTo 在多个课程出现：
  课程 09（通道缩放）→ 课程 12（线性拉伸）
```

---

## 十四、记忆口诀

```
🧠 拉伸公式：

  "减最小，除以差，乘以二五五"
  dst = (src - min) / (max - min) × 255
  用 convertTo: alpha = 255/(max-min), beta = -min*alpha

🧠 minMaxLoc 记忆：

  "最小最大和位置，四个指针全要给"
  minMaxLoc(src, &minVal, &maxVal, &minLoc, &maxLoc);
  不需要的传 nullptr

🧠 拉伸 vs 均衡化：

  "拉伸是直线，均衡是曲线"
  拉伸保持原始灰度比例关系
  均衡化重新分配，可能改变顺序
```

---

## 十五、新手雷区

```cpp
// ❌ 雷区 1：max == min 导致除零
double alpha = 255.0 / (maxVal - minVal);
// 如果图像全是同一灰度值 → maxVal == minVal → 除零！

// ✅ 正确：加保护
double range = maxVal - minVal;
if (range < 1.0) range = 1.0;  // 防止除零
double alpha = 255.0 / range;
```

```cpp
// ❌ 雷区 2：被噪点影响
cv::minMaxLoc(gray, &minVal, &maxVal);
// 一个噪点像素=0, 一个=255 → min=0, max=255 → 拉伸无效！

// ✅ 正确：用百分位截断
// 取第 1% 和第 99% 分位数作为 min/max
cv::Mat sorted;
cv::sort(gray.reshape(1, 1), sorted, cv::SORT_ASCENDING);
int total = sorted.cols;
double robustMin = sorted.at<uchar>(0, total * 0.01);
double robustMax = sorted.at<uchar>(0, total * 0.99);
```

```cpp
// ❌ 雷区 3：忘记 convertTo 的饱和截断
gray.convertTo(dst, CV_8U, alpha, beta);
// 其实不是 bug！convertTo 自动做饱和截断（超 255→255, 低于 0→0）
// 但如果输出类型写成 CV_32F，结果就不会截断

// ✅ 了解：输出 CV_8U 自动截断，CV_32F 不截断
gray.convertTo(dst, CV_8U, alpha, beta);   // 安全
gray.convertTo(dst, CV_32F, alpha, beta);  // 不截断，需手动处理
```

---

## 十六、思考题

1. **如果图像灰度范围是 [50, 200]，拉伸后的 alpha 和 beta 分别是多少？**
   提示：alpha = 255/(200-50), beta = -50 × alpha。

2. **对已经满范围 [0, 255] 的图做对比度拉伸会怎样？**
   提示：alpha=1, beta=0 → 不变。说明什么时候拉伸有意义？

3. **为什么说直方图均衡化比线性拉伸"更激进"？**
   提示：均衡化按 CDF 映射，会让密集的灰度区域展开得更多。

---

## 十七、速查卡片

```
┌─────────────────── 课程 12 速查 ───────────────────┐
│                                                    │
│  获取灰度范围:                                      │
│    double minV, maxV;                              │
│    cv::minMaxLoc(gray, &minV, &maxV);              │
│                                                    │
│  线性拉伸:                                          │
│    double alpha = 255.0 / (maxV - minV);           │
│    double beta  = -minV * alpha;                   │
│    gray.convertTo(dst, CV_8U, alpha, beta);        │
│                                                    │
│  等效的 normalize:                                  │
│    cv::normalize(gray, dst, 0, 255,                │
│                  cv::NORM_MINMAX, CV_8U);          │
│                                                    │
│  公式: dst = (src - min) × 255/(max-min)           │
│  = src × alpha + beta                              │
│                                                    │
│  ⚠️ 注意除零：max == min 时需特殊处理               │
│                                                    │
│  ── 点运算系列总结 ──                               │
│  06 Gamma    | dst = 255×(src/255)^γ               │
│  07 均衡化   | dst = CDF(src)×255                  │
│  08 截断     | dst = min(src, T)                   │
│  09 颜色     | dst = src × scale (通道)            │
│  10 反相     | dst = 255 - src                     │
│  11 二值化   | dst = src>T ? 255 : 0               │
│  12 拉伸     | dst = (src-min)×255/(max-min)       │
│                                                    │
└────────────────────────────────────────────────────┘
```

---

## 十八、延伸阅读

- [cv::minMaxLoc 文档](https://docs.opencv.org/4.x/d2/de8/group__core__array.html#gab473bf2eb6d14ff97e89b355dac20707) — 查找最大最小值
- [cv::normalize 文档](https://docs.opencv.org/4.x/d2/de8/group__core__array.html#ga87eef7ee3970f86906d0cb8ed9a2f89e) — 归一化函数
- [Mat::convertTo 文档](https://docs.opencv.org/4.x/d3/d63/classcv_1_1Mat.html#adf88c60c5b4980e05bb556080916978b) — 线性变换 alpha×src+beta
