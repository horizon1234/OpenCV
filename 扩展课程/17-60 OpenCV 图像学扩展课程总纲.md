# OpenCV 图像学扩展课程总纲（17-50）

> 目标：在现有 01-16 节基础上，把课程体系扩展到覆盖 OpenCV 图像处理主干知识。
> 说明：本总纲负责给出 17-50 节完整主线；其中 17-24 节已额外补充了独立讲义和可运行示例代码。

---

## 一、现有 01-16 节覆盖了什么

你当前仓库已经覆盖了下面几个基础模块：

1. 图像读写与窗口显示
2. 基础形态学：腐蚀、膨胀、边界提取
3. 点运算：灰度变换、直方图、截断、反相、阈值、对比度拉伸
4. 色彩空间与颜色阈值分割
5. 分割后处理与连通区域分析

这条线很适合初学者，但如果目标是做成完整的 OpenCV 图像学课程，后续至少还需要补充下面 7 大板块：

1. 邻域运算与滤波
2. 边缘、轮廓与几何形状分析
3. 几何变换与图像配准
4. 频域处理与图像复原
5. 分割进阶与图像修复
6. 特征点、匹配与拼接
7. 视频分析、跟踪与 DNN 入门

---

## 二、课程扩展总表（17-50）

### 17-24：滤波、边缘、轮廓基础

17. 邻域运算-卷积与滤波基础
- 核心 API：filter2D
- 核心原理：卷积、核、邻域、边界处理、低通和高通的直觉理解
- 实验目标：亲手定义 3x3 核并观察模糊、平滑、锐化差异

18. 平滑滤波-blur boxFilter GaussianBlur
- 核心 API：blur、boxFilter、GaussianBlur
- 核心原理：均值滤波、高斯分布、噪声平滑、边缘损失
- 实验目标：同一张带噪图比较三种平滑结果

19. 去噪滤波-medianBlur bilateralFilter
- 核心 API：medianBlur、bilateralFilter
- 核心原理：椒盐噪声、中值统计、边缘保持、空间距离和颜色距离联合权重
- 实验目标：比较中值滤波和双边滤波的适用场景

20. 锐化增强-拉普拉斯与自定义核
- 核心 API：Laplacian、filter2D、addWeighted
- 核心原理：二阶导数、细节增强、反遮罩、锐化过冲
- 实验目标：用自定义核和拉普拉斯做两种锐化

21. 梯度检测-Sobel Scharr spatialGradient
- 核心 API：Sobel、Scharr、spatialGradient、convertScaleAbs
- 核心原理：图像一阶导数、边缘方向、梯度幅值、梯度方向
- 实验目标：分离水平边缘与垂直边缘

22. 边缘检测-Canny
- 核心 API：Canny
- 核心原理：高斯去噪、梯度、非极大值抑制、双阈值连接
- 实验目标：调节高低阈值观察边缘完整度

23. 轮廓提取-findContours
- 核心 API：findContours、drawContours
- 核心原理：二值边界跟踪、轮廓层级、外轮廓与孔洞
- 实验目标：提取多个独立目标轮廓并显示编号

24. 轮廓特征-面积周长外接框
- 核心 API：contourArea、arcLength、boundingRect、minAreaRect
- 核心原理：形状几何描述、旋转包围盒、目标筛选基础
- 实验目标：基于面积和宽高比筛选候选目标

### 25-32：轮廓进阶与几何变换

25. 轮廓形状分析-凸包与缺陷
- 核心 API：convexHull、convexityDefects
- 核心原理：凸集、凹陷点、手势与不规则物体分析
- 实验目标：比较原轮廓和凸包轮廓

26. 轮廓近似-多边形拟合
- 核心 API：approxPolyDP
- 核心原理：Douglas-Peucker 简化、多边形近似、规则图形识别
- 实验目标：识别三角形、矩形、五边形

27. 矩分析-moments 与质心
- 核心 API：moments、HuMoments
- 核心原理：零阶矩、一阶矩、质心、形状不变矩
- 实验目标：计算目标重心并标注

28. 霍夫直线检测-HoughLines HoughLinesP
- 核心 API：HoughLines、HoughLinesP
- 核心原理：参数空间投票、直线极坐标表示、概率霍夫变换
- 实验目标：检测表格线、道路线

29. 霍夫圆检测-HoughCircles
- 核心 API：HoughCircles
- 核心原理：圆参数空间、累加器投票、预处理对检测稳定性的影响
- 实验目标：检测硬币或圆形标记

30. 图像缩放-resize 插值方法
- 核心 API：resize
- 核心原理：最近邻、双线性、双三次、面积插值
- 实验目标：放大文字边缘与自然图像细节比较

31. 几何变换-平移旋转翻转
- 核心 API：warpAffine、getRotationMatrix2D、flip
- 核心原理：仿射矩阵、坐标变换、插值与边界填充
- 实验目标：交互式旋转和平移演示

32. 透视变换-warpPerspective
- 核心 API：getPerspectiveTransform、warpPerspective
- 核心原理：四点定单应性、透视畸变、俯视矫正
- 实验目标：文档拍摄校正

### 33-40：多尺度、匹配、频域与复原

33. 金字塔-pyrDown pyrUp 图像多尺度
- 核心 API：pyrDown、pyrUp
- 核心原理：尺度空间、下采样、上采样、信息损失
- 实验目标：不同尺度下的边缘和噪声变化

34. 模板匹配-matchTemplate
- 核心 API：matchTemplate、minMaxLoc
- 核心原理：滑窗搜索、相似度度量、归一化相关系数
- 实验目标：在大图中定位小图标

35. 直方图进阶-多通道直方图与比较
- 核心 API：calcHist、compareHist、normalize
- 核心原理：颜色分布、相似度、交叉熵和巴氏距离直觉
- 实验目标：做简单图像检索

36. 反向投影-calcBackProject
- 核心 API：calcBackProject
- 核心原理：颜色模型反查、概率映射、区域候选生成
- 实验目标：按颜色分布查找目标

37. 傅里叶变换-DFT 频域入门
- 核心 API：dft、idft、magnitude
- 核心原理：空间域、频率域、低频表示轮廓、高频表示细节
- 实验目标：显示频谱图并理解频谱中心化

38. 频域滤波-低通高通带阻
- 核心 API：dft、mulSpectrums、idft
- 核心原理：频域去噪、平滑、细节增强、周期噪声抑制
- 实验目标：对人工周期噪声做滤波

39. 图像复原-去模糊与反卷积概念
- 核心 API：filter2D、dft
- 核心原理：PSF、卷积退化、逆滤波、维纳滤波概念
- 实验目标：模拟运动模糊后尝试恢复

40. 图像修复-inpaint
- 核心 API：inpaint
- 核心原理：掩码修复、邻域传播、纹理和结构恢复基础
- 实验目标：去除划痕或小块遮挡

### 41-46：分割进阶与可视化表达

41. 非局部去噪-fastNlMeansDenoising
- 核心 API：fastNlMeansDenoising、fastNlMeansDenoisingColored
- 核心原理：非局部相似块、纹理保留、细节与时间开销权衡
- 实验目标：与 GaussianBlur、bilateralFilter 比较

42. 距离变换-distanceTransform
- 核心 API：distanceTransform、normalize
- 核心原理：前景内部到背景的距离、骨架先验、中心区域提取
- 实验目标：提取目标中心区域

43. 分水岭分割-watershed
- 核心 API：watershed
- 核心原理：地形图、局部极小值、种子标记、过分割问题
- 实验目标：分离粘连硬币

44. GrabCut 交互式前景提取
- 核心 API：grabCut
- 核心原理：图割、前景背景建模、能量最小化直觉
- 实验目标：人物或物体抠图

45. 图像叠加-addWeighted 与透明合成
- 核心 API：addWeighted、copyTo
- 核心原理：alpha 混合、ROI 叠加、可视化表达
- 实验目标：图像融合、水印、标注叠加

46. 伪彩色-applyColorMap
- 核心 API：applyColorMap
- 核心原理：灰度到颜色映射、视觉增强、热图表达
- 实验目标：将响应图和深度图着色显示

### 47-50：特征点、匹配与视频分析入口

47. 角点检测-Harris Shi-Tomasi
- 核心 API：cornerHarris、goodFeaturesToTrack
- 核心原理：灰度变化、局部结构张量、平坦区/边缘/角点区分
- 实验目标：检测棋盘格角点

48. 特征点检测-FAST BRISK ORB
- 核心 API：FastFeatureDetector、BRISK、ORB
- 核心原理：关键点、局部不变性、描述子思想
- 实验目标：不同尺度和旋转下的稳定性比较

49. 特征匹配-BFMatcher FLANN
- 核心 API：BFMatcher、FlannBasedMatcher
- 核心原理：距离度量、KNN 匹配、比值测试、误匹配剔除
- 实验目标：两张图间做目标匹配

50. 视频读取与保存-VideoCapture VideoWriter
- 核心 API：VideoCapture、VideoWriter
- 核心原理：帧序列、编码、实时处理循环
- 实验目标：采集摄像头并实时加处理结果

---

## 三、如果你要做到“比较完整”，50 节之后还建议继续补

51. 背景建模-MOG2 KNN
52. 帧间差分-运动目标检测
53. 光流法-LK Farneback
54. CamShift MeanShift 颜色跟踪
55. 特征点拼接-homography 与 RANSAC
56. 相机标定-chessboard
57. 畸变校正-undistort
58. 双目视觉与视差图基础
59. cv::dnn 与 ONNX 推理
60. OpenCV 性能优化与工程化

---

## 四、建议教学顺序

最合理的顺序不是“想到什么讲什么”，而是沿着图像处理的因果链走：

1. 先学滤波，因为后面边缘、轮廓、霍夫都依赖预处理
2. 再学梯度和 Canny，因为它们是很多几何检测的入口
3. 再学轮廓和形状分析，因为这是从像素走向目标的第一步
4. 再学几何变换，因为很多工程任务都要做校正和映射
5. 再学频域和复原，因为这能把“滤波”从经验操作提升到原理层面
6. 最后接特征点、视频分析和 DNN，把二维图像处理扩展到真实视觉任务

---

## 五、你现在可以直接怎么做

如果要继续按照你当前仓库的方式迭代，我建议分三阶段：

1. 第 1 批：17-24，补齐滤波、梯度、边缘、轮廓
2. 第 2 批：25-32，补齐轮廓进阶与几何变换
3. 第 3 批：33-50，补齐频域、分割进阶、特征点和视频入口

这样做的好处是：

- 知识连贯
- 代码复用率高
- 每一批都可以单独形成一套教学闭环

---

## 六、配套代码说明

当前仓库已补充以下实现层资源：

- `扩展课程/advanced_lessons_demo.cpp`
- `advanced_lesson_runtime.cpp`
- `advanced_lesson_widget.cpp`
- 17-60 各课程目录下的 `*_lesson_widget.h/.cpp`

当前结构的目的很明确：

1. 首页可以像前 16 节一样跳到 17-60 节的课程页
2. 每一节课在目录结构上仍然保留自己的讲义和代码入口
3. 真正的演示算法集中在共享运行时里，避免 44 节重复维护同一类实验逻辑
