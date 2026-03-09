# 课程 16：连通区域分析——connectedComponentsWithStats

> 适合人群：已经会得到二值 mask，并想进一步判断“哪个区域才是我要的目标”

---

## 一、本课目标

这一课专门讲一件事：

> 当 mask 里有很多白色区域时，怎么把它们逐个编号、统计、筛选。

学完后你应该知道：

1. 什么叫连通区域
2. `connectedComponentsWithStats` 会返回什么
3. 怎么按面积删掉小噪声块
4. 怎么只保留最大目标
5. 怎么给每个区域画外接框和编号

---

## 二、什么是连通区域

在二值图里，如果一些白色像素彼此连在一起，它们就属于同一个连通区域。

你可以把它理解成：

- 一片独立的白色岛屿
- 一个候选目标块

所以一张 mask 往往不是“一个整体”，而是很多个白色块的集合。

---

## 三、为什么做完形态学还要做它

形态学解决的是：

- 小白点
- 小黑洞
- 毛刺

连通区域分析解决的是：

- 一共有几个候选目标
- 每个目标多大
- 哪个才值得保留

所以它们根本不是替代关系，而是前后接力。

---

## 四、核心 API

```cpp
cv::Mat labels, stats, centroids;
int numLabels = cv::connectedComponentsWithStats(
    mask,
    labels,
    stats,
    centroids
);
```

它会返回：

- `numLabels`：总标签数，包含背景
- `labels`：每个像素属于哪个标签
- `stats`：每个标签的统计信息
- `centroids`：每个标签的质心

---

## 五、stats 里最重要的字段

最常用的是这些：

- `cv::CC_STAT_LEFT`
- `cv::CC_STAT_TOP`
- `cv::CC_STAT_WIDTH`
- `cv::CC_STAT_HEIGHT`
- `cv::CC_STAT_AREA`

也就是说，你可以直接拿到：

- 外接矩形左上角
- 宽高
- 面积

---

## 六、最常见的三种用法

### 6.1 删除小面积噪声块

如果某个连通区域面积太小，就直接不要。

适合：

- 背景里有零碎误检
- 你知道真正目标不会太小

### 6.2 只保留最大连通区域

如果场景里你只关心一个主体目标，那最直接的方法常常就是：

> 找面积最大的那个。

### 6.3 给每个区域画框和编号

这很适合做教学和调试，因为你能直接看到：

- 算法到底分成了几个块
- 每个块的面积大概多大
- 误检块在哪儿

---

## 七、按面积筛选的基本模板

```cpp
cv::Mat filtered = cv::Mat::zeros(mask.size(), CV_8UC1);

for (int label = 1; label < numLabels; ++label)
{
    int area = stats.at<int>(label, cv::CC_STAT_AREA);
    if (area >= minArea)
    {
        filtered.setTo(255, labels == label);
    }
}
```

注意从 `1` 开始遍历，因为 `0` 是背景。

---

## 八、只保留最大连通区域模板

```cpp
int bestLabel = 0;
int bestArea = 0;

for (int label = 1; label < numLabels; ++label)
{
    int area = stats.at<int>(label, cv::CC_STAT_AREA);
    if (area > bestArea)
    {
        bestArea = area;
        bestLabel = label;
    }
}
```

然后只保留 `bestLabel`。

---

## 九、什么时候不要盲目保留最大块

这一步很关键。

“最大块”并不总是正确答案，比如：

- 背景误检块刚好特别大
- 真目标不止一个
- 你要的目标不是面积最大，而是位置最居中

所以连通区域分析真正的价值不是“只会找最大”，而是：

> 你终于可以基于面积、位置、宽高比等规则做筛选了。

---

## 十、调试时建议你看什么

1. 先看原始 mask 到底被分成几个块
2. 再看每个块面积是否合理
3. 看删掉小块后，误检是否明显减少
4. 看最大块是否真的就是目标
5. 最后再决定筛选规则是否要从“面积”升级到“位置 + 面积”

---

## 十一、本课小结

你至少要记住这 5 句：

1. 形态学负责“清理形状”，连通区域负责“选择目标”
2. `labels` 是像素级标签图
3. `stats` 里最重要的是面积和外接框
4. 面积筛选适合去掉小误检块
5. 最大连通区域适合单主体任务，但不是万能规则

一句话总结：

**connectedComponentsWithStats 让 mask 从“白一片”变成“一个个可统计、可筛选的候选目标”。**
