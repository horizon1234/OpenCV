# 课程 14：颜色阈值分割——inRange 的正确打开方式

> 适合人群：已经理解灰度阈值和基础色彩空间的初学者 | 建议先学课程 11 和课程 13

---

## 一、本课目标

这一课专门讲 `cv::inRange`。

学完后你应该清楚：

1. `threshold` 和 `inRange` 的区别是什么
2. 为什么灰度图常用 `threshold`，彩色分割常用 `inRange`
3. `inRange` 对单通道、三通道各自是怎么判断的
4. 为什么红色在 `HSV` 里通常要写两段区间
5. 如何把 mask 变成真正可用的分割结果
6. 为什么分割结果常常还要接形态学处理

---

## 二、先把 `threshold` 和 `inRange` 区分清楚

### 2.1 `threshold` 解决的是一个阈值

```cpp
cv::threshold(gray, binary, 128, 255, cv::THRESH_BINARY);
```

它的核心是：

> 一个值和一个阈值比大小。

通常用于：

- 灰度图二值化
- 黑白分离

---

### 2.2 `inRange` 解决的是一个区间

```cpp
cv::inRange(src, lowerb, upperb, dst);
```

它的核心是：

> 判断一个值是否落在 `[lowerb, upperb]` 这个范围里。

所以它更像：

```text
最小值 <= 当前值 <= 最大值 ?
```

---

### 2.3 如果输入是三通道会发生什么

这正是 `inRange` 最重要的地方。

比如：

```cpp
cv::inRange(hsv,
            cv::Scalar(35, 80, 50),
            cv::Scalar(85, 255, 255),
            mask);
```

意思不是“整张图只有一个范围”，而是：

- `H` 要在 `35~85`
- `S` 要在 `80~255`
- `V` 要在 `50~255`

三个条件都成立，这个像素才会进 mask。

可以写成：

$$
mask(x,y)=255 \iff
\begin{cases}
35 \le H(x,y) \le 85 \\
80 \le S(x,y) \le 255 \\
50 \le V(x,y) \le 255
\end{cases}
$$

否则就是 0。

---

## 三、inRange 的输出到底是什么

输出不是“彩色图”，而是一个**单通道 mask**：

- 满足条件的位置：`255`
- 不满足条件的位置：`0`

所以 `inRange` 的结果本质上是一个二值掩码。

```text
满足范围  -> 白色(255)
不满足    -> 黑色(0)
```

这个 mask 可以继续拿去做：

- `bitwise_and`
- 轮廓检测
- 形态学去噪
- connected components

---

## 四、最常见的流程：HSV 颜色分割

## 4.1 标准流程

```cpp
cv::Mat color = cv::imread("cat.jpg", cv::IMREAD_COLOR);

cv::Mat hsv;
cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);

cv::Mat mask;
cv::inRange(hsv,
            cv::Scalar(35, 80, 50),
            cv::Scalar(85, 255, 255),
            mask);

cv::Mat result;
cv::bitwise_and(color, color, result, mask);
```

流程图：

```text
BGR 原图
   ↓ cvtColor
HSV 图
   ↓ inRange
mask（二值图）
   ↓ bitwise_and
提取结果
```

---

## 4.2 为什么通常不是直接对 BGR inRange

可以，但通常不推荐作为第一选择。

例如：

```cpp
cv::inRange(color,
            cv::Scalar(0, 0, 80),
            cv::Scalar(120, 120, 255),
            mask);
```

这在某些场景能工作，但它更依赖：

- 光照
- 曝光
- 白平衡
- 颜色深浅

而 HSV 里你可以更清楚地控制：

- 颜色类别：`H`
- 饱和度下限：`S`
- 亮度下限：`V`

所以大多数“按颜色分割”任务，优先考虑 HSV。

---

## 五、红色是最经典的坑

## 5.1 红色为什么特殊

因为色相 `H` 是个圆。

红色跨越了首尾：

- 低端：靠近 `0`
- 高端：靠近 `179`

所以如果你只写：

```cpp
cv::inRange(hsv,
            cv::Scalar(0, 80, 50),
            cv::Scalar(12, 255, 255),
            mask);
```

你只会抓到一部分红色。

---

## 5.2 正确写法：两段区间

```cpp
cv::Mat mask1, mask2, mask;

cv::inRange(hsv,
            cv::Scalar(0, 80, 50),
            cv::Scalar(12, 255, 255),
            mask1);

cv::inRange(hsv,
            cv::Scalar(168, 80, 50),
            cv::Scalar(179, 255, 255),
            mask2);

cv::bitwise_or(mask1, mask2, mask);
```

这几乎是 HSV 红色检测的标准模板。

---

## 六、lowerb 和 upperb 怎么选

这一步是初学者最困惑的地方。

## 6.1 先定 H，再定 S/V

一个务实的顺序是：

### 第一步：先定 `H`

确定你要找什么颜色的大致色相范围。

### 第二步：给 `S` 一个下限

过滤掉灰白黑这些“颜色不鲜明”的像素。

### 第三步：给 `V` 一个下限

过滤掉太暗的噪声区域。

---

## 6.2 一个常见经验模板

```text
H：决定“是什么颜色”
S：决定“颜色得足够明显”
V：决定“别太黑”
```

比如绿色：

```cpp
lower = Scalar(35, 80, 50)
upper = Scalar(85, 255, 255)
```

解释：

- `35~85`：大致是绿色区域
- `S >= 80`：太灰的不算
- `V >= 50`：太暗的不算

---

## 七、mask 之后为什么还不够

因为真实图像里会有：

- 小白点噪声
- 小黑洞
- 边缘断裂
- 零碎误检

所以实际分割常常还要加一步形态学。

---

## 7.1 先开运算去白点噪声

```cpp
cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
cv::morphologyEx(mask, cleanMask, cv::MORPH_OPEN, kernel);
```

作用：

- 先腐蚀再膨胀
- 去掉小白点

---

## 7.2 再闭运算补小黑洞

```cpp
cv::morphologyEx(cleanMask, cleanMask, cv::MORPH_CLOSE, kernel);
```

作用：

- 先膨胀再腐蚀
- 填补目标内部的小黑洞

---

## 八、bitwise_and 到底在做什么

```cpp
cv::bitwise_and(color, color, result, mask);
```

这句的意思是：

- mask 为白的位置，保留原图像素
- mask 为黑的位置，输出黑色

所以它是“按 mask 抠出原图中的目标区域”。

---

## 九、和 lesson 11 的关系

lesson 11 的 `threshold` 是：

- 对一个灰度值做单阈值分界

lesson 14 的 `inRange` 是：

- 对一个值或一个向量做区间判断

所以关系可以这么记：

- `threshold`：单阈值
- `inRange`：双边界区间阈值

如果是 3 通道，就变成“3 组区间同时成立”。

---

## 十、实战中最常见的 4 个错误

### 错误 1：在 BGR 上死调区间

结果：

- 光照一变就失效

### 错误 2：红色只写一段区间

结果：

- 漏掉一半红色

### 错误 3：S/V 下限太低

结果：

- 背景噪声大量混进来

### 错误 4：拿到 mask 就直接结束

结果：

- 噪点多、孔洞多、边界破碎

正确思路通常是：

```text
选空间 -> 设区间 -> 得 mask -> 形态学清理 -> 再做提取或检测
```

---

## 十一、真实项目里怎么调参数最快

靠背公式不如直接开滑块。

### 11.1 最常见的调参顺序

1. 先把 `S Max`、`V Max` 拉满
2. 调 `H Min / H Max`，先把颜色大类框住
3. 再慢慢提高 `S Min`，去掉灰白杂色
4. 再慢慢提高 `V Min`，去掉暗部噪声

### 11.2 为什么滑块调参特别有用

因为 `inRange` 的难点通常不是代码，而是：

- 颜色范围到底多宽
- 饱和度下限要不要提
- 暗部阈值要不要抬高

这些参数如果只靠猜，很慢；
如果用 Trackbar，几秒钟就能看到 mask 怎么变。

### 11.3 调参时优先看什么窗口

建议顺序：

1. 先看 `mask` 是否抓到了目标
2. 再看 `result` 是否有明显漏检和误检
3. 最后再决定是否需要开闭运算

---

## 十二、推荐你掌握的最小模板

### 11.1 普通颜色分割模板

```cpp
cv::Mat color = cv::imread("cat.jpg", cv::IMREAD_COLOR);
cv::Mat hsv;
cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);

cv::Mat mask;
cv::inRange(hsv,
            cv::Scalar(35, 80, 50),
            cv::Scalar(85, 255, 255),
            mask);

cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

cv::Mat result;
cv::bitwise_and(color, color, result, mask);
```

### 11.2 红色分割模板

```cpp
cv::Mat mask1, mask2, mask;
cv::inRange(hsv, cv::Scalar(0, 80, 50), cv::Scalar(12, 255, 255), mask1);
cv::inRange(hsv, cv::Scalar(168, 80, 50), cv::Scalar(179, 255, 255), mask2);
cv::bitwise_or(mask1, mask2, mask);
```

---

## 十三、本课小结

你至少要记住这 6 句：

1. `threshold` 是单阈值，`inRange` 是区间判断
2. `inRange` 对三通道图像是“每个通道都要落在各自范围内”
3. 彩色分割优先考虑 `HSV`
4. 红色在 `HSV` 里通常需要两段区间
5. `inRange` 输出的是 mask，不是最终结果图
6. 真实任务里 mask 后面常常还要接形态学清理

最后一句：

**颜色分割真正难的不是 API，而是“用哪个空间、设什么范围、怎么清理结果”。**
