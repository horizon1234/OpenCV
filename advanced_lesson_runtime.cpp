#include "advanced_lesson_runtime.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

namespace
{
std::vector<std::string> &windowRegistry()
{
    static std::vector<std::string> names;
    return names;
}

void rememberWindow(const std::string &name)
{
    windowRegistry().push_back(name);
    cv::namedWindow(name, cv::WINDOW_NORMAL);
}

void showWindow(const std::string &name, const cv::Mat &image, int width = 520, int height = 420)
{
    rememberWindow(name);
    cv::resizeWindow(name, width, height);
    cv::imshow(name, image);
}

cv::Mat loadColorImage()
{
    return cv::imread("cat.jpg", cv::IMREAD_COLOR);
}

cv::Mat ensureColor(const cv::Mat &image)
{
    if (image.channels() == 3)
    {
        return image.clone();
    }

    cv::Mat color;
    cv::cvtColor(image, color, cv::COLOR_GRAY2BGR);
    return color;
}

cv::Mat toGray(const cv::Mat &image)
{
    if (image.channels() == 1)
    {
        return image.clone();
    }

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

cv::Mat createSyntheticCanvas(int width = 640, int height = 480)
{
    cv::Mat canvas(height, width, CV_8UC3, cv::Scalar(250, 250, 250));
    cv::rectangle(canvas, cv::Rect(70, 60, 180, 120), cv::Scalar(40, 120, 230), cv::FILLED);
    cv::circle(canvas, cv::Point(420, 160), 70, cv::Scalar(60, 180, 80), cv::FILLED);
    cv::ellipse(canvas, cv::Point(280, 340), cv::Size(110, 60), 20.0, 0, 360, cv::Scalar(210, 90, 90), cv::FILLED);
    cv::line(canvas, cv::Point(40, 420), cv::Point(600, 80), cv::Scalar(30, 30, 30), 4);
    cv::putText(canvas, "OpenCV", cv::Point(220, 430), cv::FONT_HERSHEY_SIMPLEX, 1.3, cv::Scalar(20, 20, 20), 3);
    return canvas;
}

cv::Mat addSaltPepperNoise(const cv::Mat &image, int amount = 1800)
{
    cv::Mat noisy = image.clone();
    cv::RNG rng(12345);
    for (int i = 0; i < amount; ++i)
    {
        const int y = rng.uniform(0, noisy.rows);
        const int x = rng.uniform(0, noisy.cols);
        const uchar value = (i % 2 == 0) ? 0 : 255;
        noisy.at<cv::Vec3b>(y, x) = cv::Vec3b(value, value, value);
    }
    return noisy;
}

cv::Mat addGaussianNoise(const cv::Mat &image, double sigma = 20.0)
{
    cv::Mat noise(image.size(), CV_32FC3);
    cv::randn(noise, cv::Scalar::all(0), cv::Scalar::all(sigma));

    cv::Mat floatImage;
    image.convertTo(floatImage, CV_32FC3);
    cv::Mat noisyFloat = floatImage + noise;
    cv::Mat noisy;
    noisyFloat.convertTo(noisy, CV_8UC3);
    return noisy;
}

cv::Mat drawHistogramImage(const cv::Mat &gray)
{
    int histSize = 256;
    float range[] = {0, 256};
    const float *histRange = {range};
    cv::Mat hist;
    cv::calcHist(&gray, 1, nullptr, cv::Mat(), hist, 1, &histSize, &histRange);

    const int width = 512;
    const int height = 320;
    cv::Mat histImage(height, width, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX);

    const int binWidth = cvRound(static_cast<double>(width) / histSize);
    for (int i = 1; i < histSize; ++i)
    {
        cv::line(histImage,
                 cv::Point(binWidth * (i - 1), height - cvRound(hist.at<float>(i - 1))),
                 cv::Point(binWidth * i, height - cvRound(hist.at<float>(i))),
                 cv::Scalar(70, 110, 220), 2);
    }
    return histImage;
}

cv::Mat normalizeToU8(const cv::Mat &input)
{
    cv::Mat normalized;
    cv::normalize(input, normalized, 0, 255, cv::NORM_MINMAX);
    normalized.convertTo(normalized, CV_8U);
    return normalized;
}

cv::Mat drawFlowField(const cv::Mat &frame, const cv::Mat &flow, int step = 16)
{
    cv::Mat canvas = ensureColor(frame);
    for (int y = 0; y < frame.rows; y += step)
    {
        for (int x = 0; x < frame.cols; x += step)
        {
            const cv::Point2f vector = flow.at<cv::Point2f>(y, x);
            cv::line(canvas, cv::Point(x, y), cv::Point(cvRound(x + vector.x), cvRound(y + vector.y)), cv::Scalar(0, 255, 0), 1);
            cv::circle(canvas, cv::Point(x, y), 1, cv::Scalar(0, 0, 255), -1);
        }
    }
    return canvas;
}

std::vector<cv::Mat> buildSyntheticVideoSequence()
{
    std::vector<cv::Mat> frames;
    for (int index = 0; index < 24; ++index)
    {
        cv::Mat frame(360, 640, CV_8UC3, cv::Scalar(235, 235, 235));
        cv::rectangle(frame, cv::Rect(40, 60, 160, 90), cv::Scalar(120, 120, 120), cv::FILLED);
        cv::circle(frame, cv::Point(90 + index * 16, 180), 35, cv::Scalar(40, 90, 220), cv::FILLED);
        cv::rectangle(frame, cv::Rect(320, 110 + index * 5, 80, 80), cv::Scalar(40, 200, 70), cv::FILLED);
        cv::putText(frame, "frame " + std::to_string(index), cv::Point(20, 330), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(30, 30, 30), 2);
        frames.push_back(frame);
    }
    return frames;
}

cv::Mat makePerspectiveDocument()
{
    cv::Mat canvas(500, 700, CV_8UC3, cv::Scalar(240, 240, 240));
    std::vector<cv::Point> polygon{{180, 80}, {540, 120}, {600, 380}, {130, 420}};
    cv::fillConvexPoly(canvas, polygon, cv::Scalar(255, 255, 255));
    cv::putText(canvas, "Perspective", cv::Point(220, 220), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(40, 40, 40), 2);
    cv::putText(canvas, "Warp Demo", cv::Point(250, 290), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(60, 60, 60), 2);
    return canvas;
}

cv::Mat makeChessboardImage(int squaresX = 8, int squaresY = 6, int squareSize = 48)
{
    cv::Mat board(squaresY * squareSize, squaresX * squareSize, CV_8UC1, cv::Scalar(255));
    for (int y = 0; y < squaresY; ++y)
    {
        for (int x = 0; x < squaresX; ++x)
        {
            if ((x + y) % 2 == 0)
            {
                cv::rectangle(board,
                              cv::Rect(x * squareSize, y * squareSize, squareSize, squareSize),
                              cv::Scalar(0), cv::FILLED);
            }
        }
    }
    cv::Mat color;
    cv::cvtColor(board, color, cv::COLOR_GRAY2BGR);
    return color;
}

cv::Mat computeSpectrumImage(const cv::Mat &gray)
{
    cv::Mat floatGray;
    gray.convertTo(floatGray, CV_32F);
    cv::Mat planes[] = {floatGray, cv::Mat::zeros(gray.size(), CV_32F)};
    cv::Mat complexImage;
    cv::merge(planes, 2, complexImage);
    cv::dft(complexImage, complexImage);
    cv::split(complexImage, planes);
    cv::magnitude(planes[0], planes[1], planes[0]);
    cv::Mat magnitudeImage = planes[0];
    magnitudeImage += cv::Scalar::all(1);
    cv::log(magnitudeImage, magnitudeImage);
    magnitudeImage = magnitudeImage(cv::Rect(0, 0, magnitudeImage.cols & -2, magnitudeImage.rows & -2));

    int cx = magnitudeImage.cols / 2;
    int cy = magnitudeImage.rows / 2;
    cv::Mat q0(magnitudeImage, cv::Rect(0, 0, cx, cy));
    cv::Mat q1(magnitudeImage, cv::Rect(cx, 0, cx, cy));
    cv::Mat q2(magnitudeImage, cv::Rect(0, cy, cx, cy));
    cv::Mat q3(magnitudeImage, cv::Rect(cx, cy, cx, cy));
    cv::Mat temp;
    q0.copyTo(temp);
    q3.copyTo(q0);
    temp.copyTo(q3);
    q1.copyTo(temp);
    q2.copyTo(q1);
    temp.copyTo(q2);

    return normalizeToU8(magnitudeImage);
}

cv::Mat frequencyLowPass(const cv::Mat &gray)
{
    cv::Mat floatGray;
    gray.convertTo(floatGray, CV_32F);
    cv::Mat planes[] = {floatGray, cv::Mat::zeros(gray.size(), CV_32F)};
    cv::Mat complexImage;
    cv::merge(planes, 2, complexImage);
    cv::dft(complexImage, complexImage);

    cv::Mat mask(gray.size(), CV_32FC2, cv::Scalar(0, 0));
    cv::circle(mask, cv::Point(gray.cols / 2, gray.rows / 2), std::min(gray.cols, gray.rows) / 8, cv::Scalar(1, 1), cv::FILLED);
    cv::mulSpectrums(complexImage, mask, complexImage, 0);
    cv::idft(complexImage, complexImage, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
    return normalizeToU8(complexImage);
}

cv::Mat frequencyHighPass(const cv::Mat &gray)
{
    cv::Mat low = frequencyLowPass(gray);
    cv::Mat gray8 = gray.clone();
    cv::Mat high;
    cv::subtract(gray8, low, high);
    return normalizeToU8(high);
}

std::string saveSyntheticVideo(const std::vector<cv::Mat> &frames)
{
    const std::string outputPath = "synthetic_motion_demo.avi";
    cv::VideoWriter writer(outputPath, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10.0, frames.front().size());
    if (!writer.isOpened())
    {
        return std::string();
    }

    for (const auto &frame : frames)
    {
        writer.write(frame);
    }
    return outputPath;
}

void showTriplet(const std::string &nameA, const cv::Mat &a,
                 const std::string &nameB, const cv::Mat &b,
                 const std::string &nameC, const cv::Mat &c)
{
    showWindow(nameA, a);
    showWindow(nameB, b);
    showWindow(nameC, c);
}

const std::map<int, AdvancedLessonConfig> &configs()
{
    static const std::map<int, AdvancedLessonConfig> configMap = {
        {17, {17, "课程 17：邻域运算——卷积与滤波基础", "本课用自定义卷积核把“邻域运算”讲清楚：先看均值模糊和边缘核，再看锐化核如何改变局部结构。", {{0, "均值核 vs 边缘核", "对比低通和平滑与边缘增强的直观差别"}, {1, "锐化核对比", "观察不同自定义核对细节和噪声的影响"}}}},
        {18, {18, "课程 18：平滑滤波——blur、boxFilter、GaussianBlur", "围绕 blur、boxFilter、GaussianBlur 三种最常用平滑方式，比较它们在抑噪和保边上的差异。", {{0, "三种平滑对比", "对比 blur、boxFilter、GaussianBlur 的结果"}, {1, "核尺寸影响", "观察核从小到大时平滑强度变化"}}}},
        {19, {19, "课程 19：去噪滤波——medianBlur 与 bilateralFilter", "给图像添加椒盐噪声和高斯噪声，再比较中值滤波和双边滤波的保边能力。", {{0, "椒盐噪声去除", "展示 medianBlur 处理离群噪声的效果"}, {1, "保边去噪", "比较 bilateralFilter 与普通模糊的差异"}}}},
        {20, {20, "课程 20：锐化增强——拉普拉斯与自定义核", "从 Laplacian 到 unsharp mask，观察增强细节同时如何放大噪声和边缘过冲。", {{0, "Laplacian 锐化", "查看二阶导增强细节效果"}, {1, "反遮罩增强", "对比原图、模糊图和叠加增强图"}}}},
        {21, {21, "课程 21：梯度检测——Sobel、Scharr 与 spatialGradient", "分别展示 X/Y 梯度、梯度合成和 Scharr 改进版本，帮助理解边缘方向。", {{0, "Sobel X/Y", "分离水平与垂直变化"}, {1, "Scharr 对比", "比较 Sobel 和 Scharr 在细节上的差异"}}}},
        {22, {22, "课程 22：边缘检测——Canny", "通过多组阈值和预处理对比，理解 Canny 的高低阈值如何影响边缘连通性。", {{0, "基础 Canny", "查看标准高斯预处理加 Canny 边缘图"}, {1, "阈值对比", "比较松阈值和严阈值下的边缘结果"}}}},
        {23, {23, "课程 23：轮廓提取——findContours", "在二值图上提取轮廓，比较外轮廓模式和树形层级模式，并标注轮廓编号。", {{0, "外轮廓提取", "只看最外层轮廓"}, {1, "层级轮廓", "观察孔洞和子轮廓的层级结构"}}}},
        {24, {24, "课程 24：轮廓特征——面积、周长、外接框", "围绕 contourArea、arcLength、boundingRect 和 minAreaRect 构建目标筛选最小闭环。", {{0, "外接框与面积", "给轮廓画轴对齐包围框并标面积"}, {1, "旋转框与周长", "显示旋转包围框和周长信息"}}}},
        {25, {25, "课程 25：轮廓形状分析——凸包与缺陷", "使用一组明显凹陷的合成轮廓，显示凸包和凸性缺陷最深点。", {{0, "凸包可视化", "查看原轮廓和凸包的区别"}, {1, "缺陷最深点", "标注凹陷区域的关键点"}}}},
        {26, {26, "课程 26：轮廓近似——多边形拟合", "通过 approxPolyDP 把复杂轮廓简化成多边形，并根据顶点数做基础识别。", {{0, "多边形近似", "对比原轮廓和近似轮廓"}, {1, "顶点数识别", "统计三角形、矩形等典型形状"}}}},
        {27, {27, "课程 27：矩分析——moments 与质心", "计算多个目标的质心并把 Hu 矩打印到图像上，建立“统计形状特征”的直觉。", {{0, "质心标注", "根据 moments 计算几何中心"}, {1, "Hu 矩展示", "查看轮廓的 Hu 不变矩数量级"}}}},
        {28, {28, "课程 28：霍夫直线检测——HoughLines 与 HoughLinesP", "使用合成线段图和真实边缘图，比较标准霍夫和概率霍夫线段检测。", {{0, "标准霍夫", "参数空间直线检测结果"}, {1, "概率霍夫", "直接返回线段端点"}}}},
        {29, {29, "课程 29：霍夫圆检测——HoughCircles", "在带噪圆形图上比较参数变化对圆检测稳定性的影响。", {{0, "基础圆检测", "检测多个合成圆"}, {1, "参数敏感性", "比较平滑前后和阈值变化"}}}},
        {30, {30, "课程 30：图像缩放——resize 与插值方法", "放大图像局部区域，对比最近邻、双线性、双三次和面积插值的视觉差异。", {{0, "放大插值对比", "观察文字和边缘锯齿"}, {1, "缩小插值对比", "比较缩小时混叠与清晰度"}}}},
        {31, {31, "课程 31：几何变换——平移、旋转、翻转", "通过 warpAffine 和 flip 演示坐标映射，帮助理解像素位置变化和边界填充。", {{0, "平移与旋转", "观察旋转矩阵与边界填充结果"}, {1, "翻转对比", "查看水平、垂直和双向翻转"}}}},
        {32, {32, "课程 32：透视变换——warpPerspective", "用一个斜放文档示意图演示四点透视矫正，从梯形恢复到近似正视图。", {{0, "透视矫正", "输入四边形并展开为矩形"}, {1, "点位可视化", "同时显示四个控制点和变换结果"}}}},
        {33, {33, "课程 33：金字塔——pyrDown、pyrUp 与图像多尺度", "构造多层金字塔，对比下采样和平滑效果，并观察上采样后的信息损失。", {{0, "多尺度金字塔", "连续 pyrDown 观察尺度变化"}, {1, "下采样再上采样", "比较重建图和原图差异"}}}},
        {34, {34, "课程 34：模板匹配——matchTemplate", "在大图中嵌入模板，展示响应图、最佳匹配位置和多模板干扰现象。", {{0, "模板定位", "获取最佳匹配位置"}, {1, "响应图可视化", "观察模板响应分布"}}}},
        {35, {35, "课程 35：直方图进阶——多通道直方图与比较", "比较原图、颜色扰动图和模糊图的颜色直方图相似度，理解全局特征的边界。", {{0, "直方图比较", "对比多张图的 histogram distance"}, {1, "HSV 二维直方图", "显示联合分布和归一化结果"}}}},
        {36, {36, "课程 36：反向投影——calcBackProject", "从目标 ROI 学颜色分布，再回投到整图中，形成“哪里更像目标”的概率图。", {{0, "ROI 反向投影", "根据样本区域做颜色概率图"}, {1, "阈值后轮廓", "把 backproject 结果继续变成候选区域"}}}},
        {37, {37, "课程 37：傅里叶变换——DFT 频域入门", "显示原图与频谱图，理解低频和高频在频域中的分布位置。", {{0, "频谱图", "查看 DFT 幅值谱"}, {1, "频谱与边缘", "对比高频丰富区域"}}}},
        {38, {38, "课程 38：频域滤波——低通、高通、带阻", "从频谱图出发分别做低通和高通，直观理解平滑和边缘增强在频域里的对应关系。", {{0, "低通滤波", "保留低频，查看平滑结果"}, {1, "高通滤波", "提取高频，突出边缘和纹理"}}}},
        {39, {39, "课程 39：图像复原——去模糊与反卷积概念", "先人为制造运动模糊，再做简单逆滤波近似，观察复原和噪声放大的两面性。", {{0, "模糊生成", "构造一条水平运动模糊核"}, {1, "简化逆滤波", "查看复原近似及噪声风险"}}}},
        {40, {40, "课程 40：图像修复——inpaint", "对图像添加划痕和遮挡掩码，再用 inpaint 进行局部修复。", {{0, "细划痕修复", "观察局部小缺损修复效果"}, {1, "文字遮挡修复", "对小块遮挡区域进行恢复"}}}},
        {41, {41, "课程 41：非局部去噪——fastNlMeansDenoising", "在高斯噪声条件下对比 GaussianBlur、bilateralFilter 和 NLM 的细节保留差异。", {{0, "NLM 去噪", "比较非局部去噪和普通模糊"}, {1, "彩色 NLM", "查看彩色图像纹理保留效果"}}}},
        {42, {42, "课程 42：距离变换——distanceTransform", "对粘连前景做距离变换，观察中心区域增强，并为后续分水岭准备种子。", {{0, "距离热图", "将 distanceTransform 归一化并着色"}, {1, "中心种子提取", "阈值化距离图得到前景核心区"}}}},
        {43, {43, "课程 43：分水岭分割——watershed", "以粘连硬币风格的合成图为例，完整展示阈值、距离图、markers 和分水岭边界。", {{0, "粘连目标分离", "完整演示 watershed 分割"}, {1, "markers 可视化", "显示分割前的标记图"}}}},
        {44, {44, "课程 44：GrabCut——交互式前景提取", "通过一个包含主体和背景的图像，加矩形先验后运行 GrabCut，展示掩码与抠图结果。", {{0, "矩形初始化", "使用矩形进行前景提取"}, {1, "掩码结果", "查看前景 mask 与抠图"}}}},
        {45, {45, "课程 45：图像叠加——addWeighted 与透明合成", "将边缘图、热力图与原图进行 alpha 混合，展示视觉提示层的常见做法。", {{0, "边缘叠加", "把 Canny 结果混合回原图"}, {1, "热图融合", "把伪彩色响应图叠加到原图"}}}},
        {46, {46, "课程 46：伪彩色——applyColorMap", "把灰度图、距离图和梯度响应图转成伪彩色，提升可视化可读性。", {{0, "灰度热图", "灰度图转伪彩色"}, {1, "梯度热图", "响应图着色显示"}}}},
        {47, {47, "课程 47：角点检测——Harris 与 Shi-Tomasi", "同一张图上同时显示 Harris 响应和 Shi-Tomasi 特征点，区分边缘和角点。", {{0, "Harris 角点", "显示强响应角点位置"}, {1, "Shi-Tomasi 点", "使用 goodFeaturesToTrack 选点"}}}},
        {48, {48, "课程 48：特征点检测——FAST、BRISK、ORB", "展示三类常见关键点检测或描述方案在同一张图上的关键点分布。", {{0, "FAST/BRISK/ORB 对比", "比较不同特征点分布"}, {1, "ORB 描述子点", "突出适合匹配的 ORB 关键点"}}}},
        {49, {49, "课程 49：特征匹配——BFMatcher 与 FLANN", "用 ORB 特征把原图和变换图进行匹配，并利用 KNN 和比值测试筛掉误匹配。", {{0, "BFMatcher 匹配", "展示匹配线和好匹配筛选"}, {1, "KNN 比值测试", "比较比值测试前后的匹配质量"}}}},
        {50, {50, "课程 50：视频读取与保存——VideoCapture 与 VideoWriter", "通过合成视频序列模拟视频 IO 流程，并把结果写成文件，避免依赖摄像头环境。", {{0, "写出合成视频", "生成并保存演示视频"}, {1, "读取帧预览", "用 VideoCapture 重新读取写出的文件"}}}},
        {51, {51, "课程 51：背景建模——MOG2 与 KNN", "在合成运动序列上比较 MOG2 和 KNN 背景减除结果。", {{0, "MOG2 前景", "背景减除前景 mask"}, {1, "KNN 前景", "KNN 背景模型对比"}}}},
        {52, {52, "课程 52：帧间差分——运动目标检测", "直接对连续帧做差，观察运动区域掩码和后处理轮廓。", {{0, "双帧差分", "查看差分图和二值前景"}, {1, "轮廓框选", "给运动区域画框"}}}},
        {53, {53, "课程 53：光流法——LK 与 Farneback", "同时展示稀疏光流跟踪线和稠密光流场，让“运动向量”概念真正可视化。", {{0, "LK 稀疏光流", "跟踪 Shi-Tomasi 点"}, {1, "Farneback 稠密光流", "绘制流场箭头"}}}},
        {54, {54, "课程 54：CamShift 与 MeanShift——颜色跟踪", "从颜色样本建直方图，通过 backprojection 和 CamShift 在合成序列中跟踪彩色目标。", {{0, "MeanShift 跟踪", "固定窗口追踪颜色目标"}, {1, "CamShift 跟踪", "自适应窗口大小和方向"}}}},
        {55, {55, "课程 55：特征点拼接——homography 与 RANSAC", "把原图和透视变换图做 ORB 匹配，利用 RANSAC 求 homography，并完成配准。", {{0, "RANSAC 单应性", "显示 inlier 匹配"}, {1, "warpPerspective 配准", "将目标图反变换回参考视角"}}}},
        {56, {56, "课程 56：相机标定——chessboard", "用合成棋盘格演示角点检测，并给出最小标定数据组织方式，帮助理解 object points / image points。", {{0, "棋盘角点检测", "检测内角点并绘制"}, {1, "标定数据模板", "显示 object points 与 image points 组织结果"}}}},
        {57, {57, "课程 57：畸变校正——undistort", "利用手工给定的畸变参数，对图像做 undistort 和 remap，观察直线恢复效果。", {{0, "undistort", "直接使用 undistort 校正"}, {1, "remap 校正", "展示映射表方式的校正"}}}},
        {58, {58, "课程 58：双目视觉与视差图基础", "用左右平移的合成双目图演示 StereoBM 视差估计，帮助建立“视差越大越近”的直觉。", {{0, "双目视差", "计算并显示视差图"}, {1, "视差伪彩色", "把视差图着色显示"}}}},
        {59, {59, "课程 59：cv::dnn 与 ONNX 推理", "在不依赖外部模型的前提下演示 blobFromImage、张量形状和输出后处理模板，便于后续替换真实 ONNX 模型。", {{0, "blob 预处理", "查看 blob 维度与缩放结果"}, {1, "检测框后处理模板", "模拟 NMS 和框绘制流程"}}}},
        {60, {60, "课程 60：OpenCV 性能优化与工程化", "通过 TickMeter 比较整图处理和 ROI 处理耗时，并演示中间结果与性能统计的工程化输出。", {{0, "整图 vs ROI 耗时", "比较两套处理流程耗时"}, {1, "流水线分段计时", "展示每个步骤的 TickMeter 统计"}}}},
    };
    return configMap;
}

bool lessonDemo(int lessonId, int demoId, std::string &statusMessage)
{
    cv::Mat color = loadColorImage();
    if (color.empty() && lessonId != 50 && lessonId != 51 && lessonId != 52 && lessonId != 53 && lessonId != 54 && lessonId != 56)
    {
        color = createSyntheticCanvas();
        statusMessage = "未找到 cat.jpg，已自动退回到合成图像演示。";
    }

    switch (lessonId)
    {
    case 17:
    {
        cv::Mat meanFiltered;
        cv::Mat edgeFiltered;
        cv::Mat kernelMean = cv::Mat::ones(3, 3, CV_32F) / 9.0f;
        cv::Mat kernelEdge = (cv::Mat_<float>(3, 3) << -1, -1, -1, -1, 8, -1, -1, -1, -1);
        cv::filter2D(color, meanFiltered, -1, kernelMean);
        cv::filter2D(color, edgeFiltered, CV_32F, kernelEdge);
        edgeFiltered = normalizeToU8(cv::abs(edgeFiltered));
        if (demoId == 0)
        {
            showTriplet("Lesson17 Original", color, "Lesson17 Mean Kernel", meanFiltered, "Lesson17 Edge Kernel", edgeFiltered);
            statusMessage = "卷积核并不是固定写法，而是把邻域像素重新加权。均值核更平滑，边缘核更强调变化。";
        }
        else
        {
            cv::Mat sharpened;
            cv::Mat emboss;
            cv::Mat kernelSharpen = (cv::Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
            cv::Mat kernelEmboss = (cv::Mat_<float>(3, 3) << -2, -1, 0, -1, 1, 1, 0, 1, 2);
            cv::filter2D(color, sharpened, -1, kernelSharpen);
            cv::filter2D(color, emboss, -1, kernelEmboss);
            showTriplet("Lesson17 Original", color, "Lesson17 Sharpen", sharpened, "Lesson17 Emboss", emboss);
            statusMessage = "不同卷积核本质是在改变局部邻域的权重分布。锐化核增强细节，浮雕核突出方向性变化。";
        }
        return true;
    }
    case 18:
    {
        if (demoId == 0)
        {
            cv::Mat blurImage;
            cv::Mat boxImage;
            cv::Mat gaussianImage;
            cv::blur(color, blurImage, cv::Size(9, 9));
            cv::boxFilter(color, boxImage, -1, cv::Size(9, 9));
            cv::GaussianBlur(color, gaussianImage, cv::Size(9, 9), 1.8);
            showTriplet("Lesson18 blur", blurImage, "Lesson18 boxFilter", boxImage, "Lesson18 GaussianBlur", gaussianImage);
            statusMessage = "三种平滑都能抑制高频噪声，但 GaussianBlur 对边缘过渡更自然，通常是后续边缘检测的默认预处理。";
        }
        else
        {
            cv::Mat gaussianSmall;
            cv::Mat gaussianMedium;
            cv::Mat gaussianLarge;
            cv::GaussianBlur(color, gaussianSmall, cv::Size(3, 3), 0.8);
            cv::GaussianBlur(color, gaussianMedium, cv::Size(9, 9), 1.8);
            cv::GaussianBlur(color, gaussianLarge, cv::Size(17, 17), 3.2);
            showTriplet("Lesson18 Gaussian 3x3", gaussianSmall, "Lesson18 Gaussian 9x9", gaussianMedium, "Lesson18 Gaussian 17x17", gaussianLarge);
            statusMessage = "核越大、sigma 越大，平滑越强，但边缘和纹理也会一起被抹掉。";
        }
        return true;
    }
    case 19:
    {
        cv::Mat saltPepper = addSaltPepperNoise(color);
        cv::Mat gaussianNoisy = addGaussianNoise(color, 24.0);
        if (demoId == 0)
        {
            cv::Mat medianImage;
            cv::medianBlur(saltPepper, medianImage, 5);
            showTriplet("Lesson19 Original", color, "Lesson19 SaltPepper", saltPepper, "Lesson19 medianBlur", medianImage);
            statusMessage = "中值滤波特别适合清理椒盐噪声，因为极端离群值不会像均值那样强烈拉偏结果。";
        }
        else
        {
            cv::Mat gaussianBlurred;
            cv::Mat bilateralImage;
            cv::GaussianBlur(gaussianNoisy, gaussianBlurred, cv::Size(7, 7), 1.6);
            cv::bilateralFilter(gaussianNoisy, bilateralImage, 9, 60.0, 60.0);
            showTriplet("Lesson19 Gaussian Noise", gaussianNoisy, "Lesson19 GaussianBlur", gaussianBlurred, "Lesson19 bilateralFilter", bilateralImage);
            statusMessage = "双边滤波同时看空间距离和颜色距离，更擅长保住明显边缘，但代价是计算更慢。";
        }
        return true;
    }
    case 20:
    {
        cv::Mat gray = toGray(color);
        if (demoId == 0)
        {
            cv::Mat lap16;
            cv::Mat lap8;
            cv::Mat lapColor;
            cv::Laplacian(gray, lap16, CV_16S, 3);
            cv::convertScaleAbs(lap16, lap8);
            cv::cvtColor(lap8, lapColor, cv::COLOR_GRAY2BGR);
            showTriplet("Lesson20 Gray", gray, "Lesson20 Laplacian", lap8, "Lesson20 Overlay", cv::max(color, lapColor));
            statusMessage = "Laplacian 会突出局部快速变化区域，也就是边缘和细节区，因此常被用于锐化和清晰度分析。";
        }
        else
        {
            cv::Mat blurred;
            cv::Mat detail;
            cv::Mat unsharp;
            cv::GaussianBlur(color, blurred, cv::Size(0, 0), 2.0);
            cv::addWeighted(color, 1.6, blurred, -0.6, 0.0, unsharp);
            cv::absdiff(color, blurred, detail);
            showTriplet("Lesson20 Original", color, "Lesson20 Detail Layer", detail, "Lesson20 Unsharp", unsharp);
            statusMessage = "反遮罩的思想是“原图减模糊图得到细节层，再把细节层加回去”，这是很多锐化算法的直觉基础。";
        }
        return true;
    }
    case 21:
    {
        cv::Mat gray = toGray(color);
        if (demoId == 0)
        {
            cv::Mat gradX16;
            cv::Mat gradY16;
            cv::Mat gradX8;
            cv::Mat gradY8;
            cv::Mat combined;
            cv::Sobel(gray, gradX16, CV_16S, 1, 0, 3);
            cv::Sobel(gray, gradY16, CV_16S, 0, 1, 3);
            cv::convertScaleAbs(gradX16, gradX8);
            cv::convertScaleAbs(gradY16, gradY8);
            cv::addWeighted(gradX8, 0.5, gradY8, 0.5, 0.0, combined);
            showTriplet("Lesson21 Sobel X", gradX8, "Lesson21 Sobel Y", gradY8, "Lesson21 Combined", combined);
            statusMessage = "X 梯度强调垂直边缘，Y 梯度强调水平边缘。把两者合起来，才能得到更完整的边缘强度图。";
        }
        else
        {
            cv::Mat sobel16;
            cv::Mat sobel8;
            cv::Mat scharr16;
            cv::Mat scharr8;
            cv::Sobel(gray, sobel16, CV_16S, 1, 0, 3);
            cv::Scharr(gray, scharr16, CV_16S, 1, 0);
            cv::convertScaleAbs(sobel16, sobel8);
            cv::convertScaleAbs(scharr16, scharr8);
            showTriplet("Lesson21 Gray", gray, "Lesson21 Sobel", sobel8, "Lesson21 Scharr", scharr8);
            statusMessage = "Scharr 是对小尺寸核梯度估计的改进版本，通常会比 3x3 Sobel 更稳定一些。";
        }
        return true;
    }
    case 22:
    {
        cv::Mat gray = toGray(color);
        cv::Mat blurred;
        cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.2);
        if (demoId == 0)
        {
            cv::Mat canny;
            cv::Canny(blurred, canny, 60.0, 180.0);
            showTriplet("Lesson22 Gray", gray, "Lesson22 Blurred", blurred, "Lesson22 Canny", canny);
            statusMessage = "Canny 不是单纯阈值，而是高斯平滑、梯度、非极大值抑制和双阈值连接组成的一整套边缘流程。";
        }
        else
        {
            cv::Mat loose;
            cv::Mat strict;
            cv::Canny(blurred, loose, 30.0, 90.0);
            cv::Canny(blurred, strict, 90.0, 220.0);
            showTriplet("Lesson22 Original", color, "Lesson22 Loose Threshold", loose, "Lesson22 Strict Threshold", strict);
            statusMessage = "阈值低时边缘更全但噪声也更多，阈值高时结果更干净但容易断裂。Canny 的关键就在这两者平衡。";
        }
        return true;
    }
    case 23:
    {
        cv::Mat gray = toGray(color);
        cv::Mat binary;
        cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        if (demoId == 0)
        {
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::Mat canvas = color.clone();
            for (std::size_t i = 0; i < contours.size(); ++i)
            {
                cv::drawContours(canvas, contours, static_cast<int>(i), cv::Scalar(0, 255, 0), 2);
                const cv::Rect rect = cv::boundingRect(contours[i]);
                cv::putText(canvas, std::to_string(i), rect.tl() + cv::Point(0, 18), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
            }
            showTriplet("Lesson23 Gray", gray, "Lesson23 Binary", binary, "Lesson23 External Contours", canvas);
            statusMessage = "RETR_EXTERNAL 只保留最外层轮廓，适合单层目标检测和快速计数。";
        }
        else
        {
            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> hierarchy;
            cv::findContours(binary, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
            cv::Mat canvas(binary.size(), CV_8UC3, cv::Scalar(255, 255, 255));
            for (std::size_t i = 0; i < contours.size(); ++i)
            {
                cv::Scalar drawColor(40 + (i * 37) % 180, 60 + (i * 53) % 180, 80 + (i * 71) % 180);
                cv::drawContours(canvas, contours, static_cast<int>(i), drawColor, 2);
            }
            showTriplet("Lesson23 Binary", binary, "Lesson23 Hierarchy", canvas, "Lesson23 Source", color);
            statusMessage = "RETR_TREE 会保留孔洞和子轮廓的层级关系，这在文档、零件孔洞、嵌套目标场景里很有用。";
        }
        return true;
    }
    case 24:
    {
        cv::Mat gray = toGray(color);
        cv::Mat binary;
        cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::Mat canvas = color.clone();
        if (demoId == 0)
        {
            for (const auto &contour : contours)
            {
                const double area = cv::contourArea(contour);
                if (area < 200.0)
                {
                    continue;
                }
                const cv::Rect rect = cv::boundingRect(contour);
                cv::rectangle(canvas, rect, cv::Scalar(255, 0, 0), 2);
                cv::putText(canvas, "A=" + std::to_string(static_cast<int>(area)), rect.tl() + cv::Point(0, -6), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
            }
            showTriplet("Lesson24 Binary", binary, "Lesson24 BoundingRect", canvas, "Lesson24 Source", color);
            statusMessage = "面积和轴对齐包围框是最便宜也最常用的轮廓特征，足够支撑大量基础筛选任务。";
        }
        else
        {
            for (const auto &contour : contours)
            {
                if (cv::contourArea(contour) < 200.0)
                {
                    continue;
                }
                const cv::RotatedRect rotated = cv::minAreaRect(contour);
                cv::Point2f points[4];
                rotated.points(points);
                for (int i = 0; i < 4; ++i)
                {
                    cv::line(canvas, points[i], points[(i + 1) % 4], cv::Scalar(0, 255, 255), 2);
                }
                const double perimeter = cv::arcLength(contour, true);
                cv::putText(canvas, "P=" + std::to_string(static_cast<int>(perimeter)), rotated.center, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
            }
            showTriplet("Lesson24 Binary", binary, "Lesson24 RotatedRect", canvas, "Lesson24 Source", color);
            statusMessage = "面对旋转目标时，minAreaRect 往往比 boundingRect 更稳定，因为它不再被图像坐标轴绑定。";
        }
        return true;
    }
    case 25:
    {
        cv::Mat canvas(420, 640, CV_8UC3, cv::Scalar(250, 250, 250));
        std::vector<cv::Point> contour{{140, 130}, {240, 80}, {320, 140}, {280, 200}, {340, 270}, {240, 250}, {180, 340}, {160, 230}, {90, 200}};
        std::vector<std::vector<cv::Point>> contours{contour};
        if (demoId == 0)
        {
            std::vector<cv::Point> hull;
            cv::convexHull(contour, hull);
            cv::polylines(canvas, contours, true, cv::Scalar(50, 120, 220), 3);
            cv::polylines(canvas, hull, true, cv::Scalar(40, 180, 80), 2);
            showTriplet("Lesson25 Original Contour", canvas, "Lesson25 Hull", canvas, "Lesson25 Notes", canvas);
            statusMessage = "凸包会把原轮廓中的凹陷结构“填平”，因此它很适合提取整体外轮廓和估计凸性。";
        }
        else
        {
            std::vector<int> hullIndices;
            cv::convexHull(contour, hullIndices, false, false);
            std::vector<cv::Vec4i> defects;
            cv::convexityDefects(contour, hullIndices, defects);
            cv::polylines(canvas, contours, true, cv::Scalar(50, 120, 220), 3);
            for (const auto &defect : defects)
            {
                const cv::Point farPoint = contour[defect[2]];
                cv::circle(canvas, farPoint, 6, cv::Scalar(0, 0, 255), cv::FILLED);
            }
            showWindow("Lesson25 Defects", canvas);
            statusMessage = "凸性缺陷就是原轮廓和凸包之间凹进去的部分，手势识别里常用它统计指缝。";
        }
        return true;
    }
    case 26:
    {
        cv::Mat canvas = createSyntheticCanvas();
        cv::Mat gray = toGray(canvas);
        cv::Mat binary;
        cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::Mat preview = ensureColor(binary);
        for (const auto &contour : contours)
        {
            if (cv::contourArea(contour) < 100.0)
            {
                continue;
            }
            std::vector<cv::Point> approx;
            cv::approxPolyDP(contour, approx, 0.03 * cv::arcLength(contour, true), true);
            cv::drawContours(preview, std::vector<std::vector<cv::Point>>{approx}, -1, cv::Scalar(0, 255, 0), 2);
            cv::Rect rect = cv::boundingRect(approx);
            if (demoId == 1)
            {
                cv::putText(preview, std::to_string(approx.size()) + " pts", rect.tl() + cv::Point(0, 18), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
            }
        }
        showTriplet("Lesson26 Binary", binary, "Lesson26 Approx", preview, "Lesson26 Source", canvas);
        statusMessage = (demoId == 0)
            ? "approxPolyDP 会在保持形状大体不变的前提下删掉冗余点，让复杂轮廓变成更易分析的多边形。"
            : "顶点数只是第一层粗分类，工程里通常还要结合面积、角度和宽高比一起判断形状。";
        return true;
    }
    case 27:
    {
        cv::Mat canvas = createSyntheticCanvas();
        cv::Mat gray = toGray(canvas);
        cv::Mat binary;
        cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::Mat preview = canvas.clone();
        int row = 30;
        for (const auto &contour : contours)
        {
            if (cv::contourArea(contour) < 100.0)
            {
                continue;
            }
            cv::Moments moments = cv::moments(contour);
            const int centerX = static_cast<int>(moments.m10 / std::max(1e-5, moments.m00));
            const int centerY = static_cast<int>(moments.m01 / std::max(1e-5, moments.m00));
            cv::circle(preview, cv::Point(centerX, centerY), 5, cv::Scalar(0, 0, 255), cv::FILLED);
            if (demoId == 1)
            {
                double hu[7];
                cv::HuMoments(moments, hu);
                cv::putText(preview, "Hu0=" + cv::format("%.2e", hu[0]), cv::Point(10, row), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(20, 20, 20), 1);
                row += 18;
            }
        }
        showTriplet("Lesson27 Binary", binary, "Lesson27 Moments", preview, "Lesson27 Source", canvas);
        statusMessage = (demoId == 0)
            ? "零阶矩近似面积，一阶矩决定质心位置。通过 moments，你可以把轮廓从“边界点集合”转成统计特征。"
            : "Hu 矩提供了一组经典不变特征，适合教学中理解“形状特征抽象”这个概念。";
        return true;
    }
    case 28:
    {
        cv::Mat synthetic(420, 640, CV_8UC3, cv::Scalar(255, 255, 255));
        cv::line(synthetic, cv::Point(60, 70), cv::Point(580, 70), cv::Scalar(0, 0, 0), 3);
        cv::line(synthetic, cv::Point(120, 90), cv::Point(250, 360), cv::Scalar(0, 0, 0), 3);
        cv::line(synthetic, cv::Point(350, 60), cv::Point(580, 280), cv::Scalar(0, 0, 0), 3);
        cv::Mat edges;
        cv::Canny(toGray(synthetic), edges, 50.0, 150.0);
        cv::Mat preview = synthetic.clone();
        if (demoId == 0)
        {
            std::vector<cv::Vec2f> lines;
            cv::HoughLines(edges, lines, 1, CV_PI / 180, 120);
            for (const auto &line : lines)
            {
                float rho = line[0];
                float theta = line[1];
                double a = std::cos(theta);
                double b = std::sin(theta);
                double x0 = a * rho;
                double y0 = b * rho;
                cv::Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
                cv::Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));
                cv::line(preview, pt1, pt2, cv::Scalar(0, 0, 255), 2);
            }
            showTriplet("Lesson28 Synthetic", synthetic, "Lesson28 Canny", edges, "Lesson28 HoughLines", preview);
            statusMessage = "标准霍夫变换更接近理论表达，它返回的是极坐标参数，而不是直接的线段端点。";
        }
        else
        {
            std::vector<cv::Vec4i> linesP;
            cv::HoughLinesP(edges, linesP, 1, CV_PI / 180, 60, 60, 12);
            for (const auto &line : linesP)
            {
                cv::line(preview, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), cv::Scalar(0, 150, 255), 3);
            }
            showTriplet("Lesson28 Synthetic", synthetic, "Lesson28 Canny", edges, "Lesson28 HoughLinesP", preview);
            statusMessage = "概率霍夫直接输出线段端点，更适合工程里做道路线、表格线和边界线检测。";
        }
        return true;
    }
    case 29:
    {
        cv::Mat circles(420, 640, CV_8UC3, cv::Scalar(245, 245, 245));
        cv::circle(circles, cv::Point(170, 160), 70, cv::Scalar(90, 90, 90), 4);
        cv::circle(circles, cv::Point(350, 220), 50, cv::Scalar(90, 90, 90), 4);
        cv::circle(circles, cv::Point(510, 150), 85, cv::Scalar(90, 90, 90), 4);
        cv::Mat gray = toGray(circles);
        cv::medianBlur(gray, gray, 5);
        std::vector<cv::Vec3f> found;
        cv::HoughCircles(gray, found, cv::HOUGH_GRADIENT, 1.2, 80.0, 120.0, demoId == 0 ? 28.0 : 20.0, 30, 100);
        cv::Mat preview = circles.clone();
        for (const auto &circle : found)
        {
            cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
            int radius = cvRound(circle[2]);
            cv::circle(preview, center, radius, cv::Scalar(0, 180, 80), 2);
            cv::circle(preview, center, 3, cv::Scalar(0, 0, 255), cv::FILLED);
        }
        showTriplet("Lesson29 Synthetic", circles, "Lesson29 Gray", gray, demoId == 0 ? "Lesson29 HoughCircles" : "Lesson29 Sensitive Params", preview);
        statusMessage = (demoId == 0)
            ? "圆检测比直线检测更依赖干净边缘和合适参数，因为它的参数空间维度更高。"
            : "阈值调得过松时，圆检测会更容易误检；调得过严时，又会漏掉真实圆。";
        return true;
    }
    case 30:
    {
        cv::Rect roi(180, 120, 120, 120);
        cv::Mat patch = color(roi).clone();
        if (demoId == 0)
        {
            cv::Mat nearest;
            cv::Mat linear;
            cv::Mat cubic;
            cv::resize(patch, nearest, cv::Size(), 3.0, 3.0, cv::INTER_NEAREST);
            cv::resize(patch, linear, cv::Size(), 3.0, 3.0, cv::INTER_LINEAR);
            cv::resize(patch, cubic, cv::Size(), 3.0, 3.0, cv::INTER_CUBIC);
            showTriplet("Lesson30 Nearest", nearest, "Lesson30 Linear", linear, "Lesson30 Cubic", cubic);
            statusMessage = "放大时，最近邻最锐但锯齿明显；双线性更平衡；双三次通常更平滑自然。";
        }
        else
        {
            cv::Mat nearest;
            cv::Mat area;
            cv::Mat linear;
            cv::resize(color, nearest, cv::Size(), 0.35, 0.35, cv::INTER_NEAREST);
            cv::resize(color, area, cv::Size(), 0.35, 0.35, cv::INTER_AREA);
            cv::resize(color, linear, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
            showTriplet("Lesson30 Nearest Down", nearest, "Lesson30 Area Down", area, "Lesson30 Linear Down", linear);
            statusMessage = "缩小时，面积插值通常比线性和最近邻更稳，更不容易出现明显混叠。";
        }
        return true;
    }
    case 31:
    {
        if (demoId == 0)
        {
            cv::Mat translated;
            cv::Mat rotated;
            cv::Mat matrixTranslate = (cv::Mat_<double>(2, 3) << 1, 0, 50, 0, 1, 35);
            cv::warpAffine(color, translated, matrixTranslate, color.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(245, 245, 245));
            cv::Mat matrixRotate = cv::getRotationMatrix2D(cv::Point2f(color.cols / 2.0f, color.rows / 2.0f), 18.0, 1.0);
            cv::warpAffine(color, rotated, matrixRotate, color.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(245, 245, 245));
            showTriplet("Lesson31 Original", color, "Lesson31 Translate", translated, "Lesson31 Rotate", rotated);
            statusMessage = "几何变换改的是像素位置而不是像素值。平移和旋转本质上都是坐标映射再插值。";
        }
        else
        {
            cv::Mat flipX;
            cv::Mat flipY;
            cv::Mat flipBoth;
            cv::flip(color, flipX, 0);
            cv::flip(color, flipY, 1);
            cv::flip(color, flipBoth, -1);
            showTriplet("Lesson31 Flip X", flipX, "Lesson31 Flip Y", flipY, "Lesson31 Flip Both", flipBoth);
            statusMessage = "翻转是最简单的几何变换，但它非常适合做数据增强和左右结构对比。";
        }
        return true;
    }
    case 32:
    {
        cv::Mat doc = makePerspectiveDocument();
        std::vector<cv::Point2f> src{{180.0f, 80.0f}, {540.0f, 120.0f}, {600.0f, 380.0f}, {130.0f, 420.0f}};
        std::vector<cv::Point2f> dst{{0.0f, 0.0f}, {399.0f, 0.0f}, {399.0f, 499.0f}, {0.0f, 499.0f}};
        cv::Mat matrix = cv::getPerspectiveTransform(src, dst);
        cv::Mat corrected;
        cv::warpPerspective(doc, corrected, matrix, cv::Size(400, 500));
        if (demoId == 0)
        {
            showTriplet("Lesson32 Input", doc, "Lesson32 Rectified", corrected, "Lesson32 Gray", toGray(corrected));
            statusMessage = "透视变换适合把斜拍的平面目标矫正成正视图，是文档扫描、牌照矫正等任务的基础。";
        }
        else
        {
            cv::Mat preview = doc.clone();
            for (const auto &point : src)
            {
                cv::circle(preview, point, 8, cv::Scalar(0, 0, 255), cv::FILLED);
            }
            showTriplet("Lesson32 Source Points", preview, "Lesson32 Rectified", corrected, "Lesson32 Source", doc);
            statusMessage = "四点对应决定单应矩阵。教学时把控制点一起画出来，能帮助理解“梯形如何被拉回矩形”。";
        }
        return true;
    }
    case 33:
    {
        if (demoId == 0)
        {
            cv::Mat level1;
            cv::Mat level2;
            cv::pyrDown(color, level1);
            cv::pyrDown(level1, level2);
            showTriplet("Lesson33 Original", color, "Lesson33 pyrDown x1", level1, "Lesson33 pyrDown x2", level2);
            statusMessage = "图像金字塔不是简单 resize，它包含平滑和下采样，适合多尺度检测和分析。";
        }
        else
        {
            cv::Mat down;
            cv::Mat up;
            cv::pyrDown(color, down);
            cv::pyrUp(down, up, color.size());
            cv::Mat diff;
            cv::absdiff(color, up, diff);
            showTriplet("Lesson33 Original", color, "Lesson33 Reconstructed", up, "Lesson33 Difference", diff);
            statusMessage = "下采样后再上采样并不能无损恢复原图，因为高频细节在下采样时已经丢失了。";
        }
        return true;
    }
    case 34:
    {
        cv::Mat scene(420, 680, CV_8UC3, cv::Scalar(240, 240, 240));
        cv::Mat templ = color(cv::Rect(160, 120, 110, 90)).clone();
        templ.copyTo(scene(cv::Rect(380, 210, templ.cols, templ.rows)));
        if (demoId == 0)
        {
            cv::Mat response;
            cv::matchTemplate(scene, templ, response, cv::TM_CCOEFF_NORMED);
            double maxValue = 0.0;
            cv::Point maxPoint;
            cv::minMaxLoc(response, nullptr, &maxValue, nullptr, &maxPoint);
            cv::Mat preview = scene.clone();
            cv::rectangle(preview, cv::Rect(maxPoint.x, maxPoint.y, templ.cols, templ.rows), cv::Scalar(0, 0, 255), 2);
            showTriplet("Lesson34 Scene", scene, "Lesson34 Template", templ, "Lesson34 Match", preview);
            statusMessage = "模板匹配是滑窗搜索。响应最强的位置通常就是最像模板的位置，但它对尺度和旋转变化很敏感。";
        }
        else
        {
            cv::Mat response;
            cv::matchTemplate(scene, templ, response, cv::TM_CCOEFF_NORMED);
            cv::Mat responseVis = normalizeToU8(response);
            cv::applyColorMap(responseVis, responseVis, cv::COLORMAP_JET);
            showTriplet("Lesson34 Scene", scene, "Lesson34 Template", templ, "Lesson34 Response", responseVis);
            statusMessage = "响应图能帮助理解模板匹配的本质：它输出的不是“答案”，而是一张相似度分布图。";
        }
        return true;
    }
    case 35:
    {
        cv::Mat shifted;
        cv::Mat hsv = color.clone();
        cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);
        std::vector<cv::Mat> channels;
        cv::split(hsv, channels);
        channels[0] += 20;
        cv::merge(channels, hsv);
        cv::cvtColor(hsv, shifted, cv::COLOR_HSV2BGR);
        cv::Mat blurred;
        cv::GaussianBlur(color, blurred, cv::Size(13, 13), 2.0);

        cv::Mat histBase;
        cv::Mat histShifted;
        cv::Mat histBlurred;
        int histSize[] = {30, 32};
        float hRange[] = {0, 180};
        float sRange[] = {0, 256};
        const float *ranges[] = {hRange, sRange};
        int channelsIndex[] = {0, 1};
        cv::calcHist(&hsv, 1, channelsIndex, cv::Mat(), histBase, 2, histSize, ranges);
        cv::Mat shiftedHsv;
        cv::cvtColor(shifted, shiftedHsv, cv::COLOR_BGR2HSV);
        cv::calcHist(&shiftedHsv, 1, channelsIndex, cv::Mat(), histShifted, 2, histSize, ranges);
        cv::Mat blurredHsv;
        cv::cvtColor(blurred, blurredHsv, cv::COLOR_BGR2HSV);
        cv::calcHist(&blurredHsv, 1, channelsIndex, cv::Mat(), histBlurred, 2, histSize, ranges);
        cv::normalize(histBase, histBase, 0, 1, cv::NORM_MINMAX);
        cv::normalize(histShifted, histShifted, 0, 1, cv::NORM_MINMAX);
        cv::normalize(histBlurred, histBlurred, 0, 1, cv::NORM_MINMAX);

        cv::Mat preview = color.clone();
        if (demoId == 0)
        {
            const double corrShifted = cv::compareHist(histBase, histShifted, cv::HISTCMP_CORREL);
            const double corrBlurred = cv::compareHist(histBase, histBlurred, cv::HISTCMP_CORREL);
            cv::putText(preview, "corr(base, hue-shifted)=" + cv::format("%.2f", corrShifted), cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
            cv::putText(preview, "corr(base, blurred)=" + cv::format("%.2f", corrBlurred), cv::Point(20, 72), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
            showTriplet("Lesson35 Base", color, "Lesson35 Hue Shifted", shifted, "Lesson35 Compare", preview);
            statusMessage = "直方图比较更关注全局颜色分布，而不关心像素具体在哪儿。这既是它的优势，也是它的局限。";
        }
        else
        {
            cv::Mat gray = toGray(color);
            cv::Mat histVis = drawHistogramImage(gray);
            cv::Mat shiftedGray = toGray(shifted);
            cv::Mat shiftedHistVis = drawHistogramImage(shiftedGray);
            cv::Mat blurredHistVis = drawHistogramImage(toGray(blurred));
            showTriplet("Lesson35 Hist Base", histVis, "Lesson35 Hist Shifted", shiftedHistVis, "Lesson35 Hist Blurred", blurredHistVis);
            statusMessage = "看直方图时，颜色变化会改变分布位置，模糊变化则更常影响局部结构而不一定显著改变全局统计。";
        }
        return true;
    }
    case 36:
    {
        cv::Rect sampleRect(color.cols / 3, color.rows / 4, color.cols / 5, color.rows / 4);
        cv::Mat hsv;
        cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);
        cv::Mat roi = hsv(sampleRect);
        int histSize[] = {30, 32};
        float hRange[] = {0, 180};
        float sRange[] = {0, 256};
        const float *ranges[] = {hRange, sRange};
        int channelsIndex[] = {0, 1};
        cv::Mat hist;
        cv::calcHist(&roi, 1, channelsIndex, cv::Mat(), hist, 2, histSize, ranges);
        cv::normalize(hist, hist, 0, 255, cv::NORM_MINMAX);
        cv::Mat backProjection;
        cv::calcBackProject(&hsv, 1, channelsIndex, hist, backProjection, ranges, 1.0);
        if (demoId == 0)
        {
            cv::Mat preview = color.clone();
            cv::rectangle(preview, sampleRect, cv::Scalar(0, 0, 255), 2);
            showTriplet("Lesson36 Source ROI", preview, "Lesson36 BackProject", backProjection, "Lesson36 Heatmap", ensureColor(normalizeToU8(backProjection)));
            statusMessage = "反向投影会把“目标颜色模型”映射回整张图，得到一张“哪里更像它”的概率图。";
        }
        else
        {
            cv::Mat thresholded;
            cv::threshold(backProjection, thresholded, 120, 255, cv::THRESH_BINARY);
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7));
            cv::morphologyEx(thresholded, thresholded, cv::MORPH_OPEN, kernel);
            cv::Mat preview = color.clone();
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(thresholded, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            for (const auto &contour : contours)
            {
                if (cv::contourArea(contour) < 200.0)
                {
                    continue;
                }
                cv::rectangle(preview, cv::boundingRect(contour), cv::Scalar(0, 255, 0), 2);
            }
            showTriplet("Lesson36 BackProject", backProjection, "Lesson36 Threshold", thresholded, "Lesson36 Candidate Regions", preview);
            statusMessage = "反向投影常常不是最终结果，而是粗定位。后面还需要阈值、形态学或跟踪器继续收敛。";
        }
        return true;
    }
    case 37:
    {
        cv::Mat gray = toGray(color);
        cv::Mat spectrum = computeSpectrumImage(gray);
        if (demoId == 0)
        {
            showTriplet("Lesson37 Gray", gray, "Lesson37 Spectrum", spectrum, "Lesson37 Histogram", drawHistogramImage(gray));
            statusMessage = "频谱图把图像从“空间”换到“频率”角度观察。低频更多对应整体轮廓，高频更多对应边缘和细纹理。";
        }
        else
        {
            cv::Mat edges;
            cv::Canny(gray, edges, 60, 180);
            showTriplet("Lesson37 Gray", gray, "Lesson37 Edges", edges, "Lesson37 Spectrum", spectrum);
            statusMessage = "当图像里边缘和纹理更丰富时，频谱中的高频分量通常也会更明显。";
        }
        return true;
    }
    case 38:
    {
        cv::Mat gray = toGray(color);
        if (demoId == 0)
        {
            cv::Mat low = frequencyLowPass(gray);
            showTriplet("Lesson38 Gray", gray, "Lesson38 Spectrum", computeSpectrumImage(gray), "Lesson38 Low Pass", low);
            statusMessage = "低通滤波保留低频，结果通常更平滑；这和空间域模糊是同一件事在频域里的表达。";
        }
        else
        {
            cv::Mat high = frequencyHighPass(gray);
            showTriplet("Lesson38 Gray", gray, "Lesson38 Spectrum", computeSpectrumImage(gray), "Lesson38 High Pass", high);
            statusMessage = "高通滤波保留高频，更像是在提取边缘和细节；但它也会把噪声一起放大。";
        }
        return true;
    }
    case 39:
    {
        cv::Mat kernel = cv::Mat::zeros(1, 21, CV_32F);
        kernel.setTo(1.0f / 21.0f);
        cv::Mat blurred;
        cv::filter2D(color, blurred, -1, kernel);
        if (demoId == 0)
        {
            showTriplet("Lesson39 Original", color, "Lesson39 Motion Blur", blurred, "Lesson39 Blur Hist", drawHistogramImage(toGray(blurred)));
            statusMessage = "运动模糊可以用一条方向性 PSF 近似。复原问题的第一步，是先把“模糊”建模出来。";
        }
        else
        {
            cv::Mat sharpened;
            cv::addWeighted(color, 1.6, blurred, -0.6, 0.0, sharpened);
            showTriplet("Lesson39 Blurred", blurred, "Lesson39 Approx Restore", sharpened, "Lesson39 Original", color);
            statusMessage = "真正的反卷积比这个演示复杂得多。这个近似恢复主要是帮助理解：逆过程通常会把噪声和伪影一起放大。";
        }
        return true;
    }
    case 40:
    {
        cv::Mat damaged = color.clone();
        cv::Mat mask(color.size(), CV_8UC1, cv::Scalar(0));
        cv::line(damaged, cv::Point(40, 60), cv::Point(color.cols - 60, color.rows - 80), cv::Scalar(255, 255, 255), 5);
        cv::line(mask, cv::Point(40, 60), cv::Point(color.cols - 60, color.rows - 80), cv::Scalar(255), 5);
        if (demoId == 1)
        {
            cv::rectangle(damaged, cv::Rect(color.cols / 2 - 80, color.rows / 2 - 40, 160, 60), cv::Scalar(255, 255, 255), cv::FILLED);
            cv::rectangle(mask, cv::Rect(color.cols / 2 - 80, color.rows / 2 - 40, 160, 60), cv::Scalar(255), cv::FILLED);
        }
        cv::Mat repaired;
        cv::inpaint(damaged, mask, repaired, 3.0, cv::INPAINT_TELEA);
        showTriplet("Lesson40 Damaged", damaged, "Lesson40 Mask", mask, "Lesson40 Inpaint", repaired);
        statusMessage = (demoId == 0)
            ? "inpaint 更适合小范围细划痕修复，它依赖周围正常区域的信息向内传播。"
            : "当缺损区域变大时，传统 inpaint 的效果会明显下降，因为它没有真正理解语义结构。";
        return true;
    }
    case 41:
    {
        cv::Mat noisy = addGaussianNoise(color, 25.0);
        if (demoId == 0)
        {
            cv::Mat gaussian;
            cv::Mat nlm;
            cv::GaussianBlur(noisy, gaussian, cv::Size(9, 9), 1.8);
            cv::fastNlMeansDenoisingColored(noisy, nlm, 8.0f, 8.0f, 7, 21);
            showTriplet("Lesson41 Noisy", noisy, "Lesson41 Gaussian", gaussian, "Lesson41 NLM", nlm);
            statusMessage = "NLM 的核心不是只看近邻，而是找全图范围内“长得像”的小块一起去噪，因此更有机会保住纹理。";
        }
        else
        {
            cv::Mat bilateral;
            cv::Mat nlm;
            cv::bilateralFilter(noisy, bilateral, 9, 60, 60);
            cv::fastNlMeansDenoisingColored(noisy, nlm, 10.0f, 10.0f, 7, 21);
            showTriplet("Lesson41 Noisy", noisy, "Lesson41 bilateral", bilateral, "Lesson41 NLM", nlm);
            statusMessage = "双边滤波和 NLM 都在尝试保边，但 NLM 更像“相似块协同估计”，通常细节保留更自然。";
        }
        return true;
    }
    case 42:
    {
        cv::Mat binary(360, 480, CV_8UC1, cv::Scalar(0));
        cv::circle(binary, cv::Point(150, 160), 80, cv::Scalar(255), cv::FILLED);
        cv::circle(binary, cv::Point(300, 190), 90, cv::Scalar(255), cv::FILLED);
        cv::Mat distance;
        cv::distanceTransform(binary, distance, cv::DIST_L2, 5);
        cv::Mat distance8 = normalizeToU8(distance);
        if (demoId == 0)
        {
            cv::Mat heat;
            cv::applyColorMap(distance8, heat, cv::COLORMAP_JET);
            showTriplet("Lesson42 Binary", binary, "Lesson42 Distance", distance8, "Lesson42 Heatmap", heat);
            statusMessage = "距离变换把块状前景变成“中心高、边缘低”的强度图，非常适合做前景核心提取。";
        }
        else
        {
            cv::Mat seeds;
            cv::threshold(distance8, seeds, 170, 255, cv::THRESH_BINARY);
            showTriplet("Lesson42 Binary", binary, "Lesson42 Distance", distance8, "Lesson42 Seeds", seeds);
            statusMessage = "对距离图再阈值化，就能得到分水岭常用的“确定前景种子”。";
        }
        return true;
    }
    case 43:
    {
        cv::Mat image(360, 520, CV_8UC3, cv::Scalar(255, 255, 255));
        cv::circle(image, cv::Point(180, 180), 90, cv::Scalar(120, 120, 120), cv::FILLED);
        cv::circle(image, cv::Point(270, 190), 90, cv::Scalar(150, 150, 150), cv::FILLED);
        cv::Mat gray = toGray(image);
        cv::Mat binary;
        cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
        cv::Mat dist;
        cv::distanceTransform(binary, dist, cv::DIST_L2, 5);
        cv::Mat dist8 = normalizeToU8(dist);
        cv::Mat sureFg;
        cv::threshold(dist8, sureFg, 170, 255, cv::THRESH_BINARY);
        cv::Mat markers;
        cv::connectedComponents(sureFg, markers);
        markers += 1;
        markers.setTo(0, binary == 0);
        cv::Mat watershedInput = image.clone();
        cv::watershed(watershedInput, markers);
        if (demoId == 0)
        {
            cv::Mat result = image.clone();
            result.setTo(cv::Scalar(0, 0, 255), markers == -1);
            showTriplet("Lesson43 Binary", binary, "Lesson43 SureFG", sureFg, "Lesson43 Watershed", result);
            statusMessage = "分水岭把粘连目标拆开的关键不在复杂参数，而在是否能构造出合理的 markers。";
        }
        else
        {
            cv::Mat markers8 = normalizeToU8(markers);
            showTriplet("Lesson43 Distance", dist8, "Lesson43 Markers", markers8, "Lesson43 Source", image);
            statusMessage = "markers 可以理解成“从哪里开始注水”的先验标签。没有好种子，分水岭就很容易过分割。";
        }
        return true;
    }
    case 44:
    {
        cv::Mat image = createSyntheticCanvas();
        cv::Rect rect(90, 70, 430, 270);
        cv::Mat mask(image.size(), CV_8UC1, cv::Scalar(cv::GC_BGD));
        cv::Mat bgModel;
        cv::Mat fgModel;
        cv::grabCut(image, mask, rect, bgModel, fgModel, 4, cv::GC_INIT_WITH_RECT);
        cv::Mat foregroundMask = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);
        cv::Mat extracted(image.size(), image.type(), cv::Scalar::all(0));
        image.copyTo(extracted, foregroundMask);
        if (demoId == 0)
        {
            cv::Mat preview = image.clone();
            cv::rectangle(preview, rect, cv::Scalar(0, 0, 255), 2);
            showTriplet("Lesson44 Source", preview, "Lesson44 Mask", foregroundMask * 255, "Lesson44 Extracted", extracted);
            statusMessage = "GrabCut 结合颜色模型和邻接一致性来分前景背景，通常会比简单阈值得到更自然的边界。";
        }
        else
        {
            cv::Mat overlay = image.clone();
            overlay.setTo(cv::Scalar(60, 220, 60), foregroundMask);
            cv::addWeighted(image, 0.6, overlay, 0.4, 0.0, overlay);
            showTriplet("Lesson44 Source", image, "Lesson44 Overlay", overlay, "Lesson44 Extracted", extracted);
            statusMessage = "GrabCut 的初始矩形只提供一个粗先验，最终结果会在多轮迭代中逐步收敛。";
        }
        return true;
    }
    case 45:
    {
        cv::Mat gray = toGray(color);
        cv::Mat edges;
        cv::Canny(gray, edges, 60, 180);
        if (demoId == 0)
        {
            cv::Mat edgesColor;
            cv::cvtColor(edges, edgesColor, cv::COLOR_GRAY2BGR);
            cv::Mat overlay;
            cv::addWeighted(color, 0.8, edgesColor, 0.7, 0.0, overlay);
            showTriplet("Lesson45 Source", color, "Lesson45 Edges", edges, "Lesson45 Overlay", overlay);
            statusMessage = "addWeighted 最适合做提示层融合：你既能保留原图语义，又能把算法结果清楚叠上去。";
        }
        else
        {
            cv::Mat heat;
            cv::applyColorMap(normalizeToU8(gray), heat, cv::COLORMAP_TURBO);
            cv::Mat overlay;
            cv::addWeighted(color, 0.55, heat, 0.45, 0.0, overlay);
            showTriplet("Lesson45 Source", color, "Lesson45 Heat", heat, "Lesson45 Blend", overlay);
            statusMessage = "整图热图融合适合响应图、注意力图和深度图展示；ROI 融合则更适合局部提示和水印。";
        }
        return true;
    }
    case 46:
    {
        cv::Mat gray = toGray(color);
        if (demoId == 0)
        {
            cv::Mat heat;
            cv::applyColorMap(gray, heat, cv::COLORMAP_TURBO);
            showTriplet("Lesson46 Gray", gray, "Lesson46 Turbo", heat, "Lesson46 Source", color);
            statusMessage = "伪彩色并没有增加图像信息，但它能显著提升人眼对灰度变化的感知能力。";
        }
        else
        {
            cv::Mat sobel16;
            cv::Mat sobel8;
            cv::Sobel(gray, sobel16, CV_16S, 1, 1, 3);
            cv::convertScaleAbs(sobel16, sobel8);
            cv::Mat heat;
            cv::applyColorMap(sobel8, heat, cv::COLORMAP_JET);
            showTriplet("Lesson46 Gradient", sobel8, "Lesson46 Heat", heat, "Lesson46 Source", color);
            statusMessage = "梯度图、距离图、视差图这类“值域响应图”非常适合用伪彩色可视化。";
        }
        return true;
    }
    case 47:
    {
        cv::Mat gray = toGray(color);
        if (demoId == 0)
        {
            cv::Mat harris;
            cv::cornerHarris(gray, harris, 2, 3, 0.04);
            cv::Mat harris8 = normalizeToU8(harris);
            cv::Mat preview = color.clone();
            for (int y = 0; y < harris.rows; ++y)
            {
                for (int x = 0; x < harris.cols; ++x)
                {
                    if (harris.at<float>(y, x) > 0.02f * static_cast<float>(cv::norm(harris, cv::NORM_INF)))
                    {
                        cv::circle(preview, cv::Point(x, y), 2, cv::Scalar(0, 0, 255), -1);
                    }
                }
            }
            showTriplet("Lesson47 Gray", gray, "Lesson47 Harris Response", harris8, "Lesson47 Harris Points", preview);
            statusMessage = "角点比边缘更适合跟踪，因为它在两个方向上都有明显灰度变化。";
        }
        else
        {
            std::vector<cv::Point2f> corners;
            cv::goodFeaturesToTrack(gray, corners, 120, 0.01, 10.0);
            cv::Mat preview = color.clone();
            for (const auto &corner : corners)
            {
                cv::circle(preview, corner, 4, cv::Scalar(0, 255, 0), -1);
            }
            showTriplet("Lesson47 Gray", gray, "Lesson47 Source", color, "Lesson47 ShiTomasi", preview);
            statusMessage = "Shi-Tomasi 更偏“挑出稳定可跟踪点”，因此在 LK 光流前经常搭配 goodFeaturesToTrack 使用。";
        }
        return true;
    }
    case 48:
    {
        cv::Mat gray = toGray(color);
        if (demoId == 0)
        {
            auto fast = cv::FastFeatureDetector::create(35);
            auto brisk = cv::BRISK::create();
            std::vector<cv::KeyPoint> fastPoints;
            std::vector<cv::KeyPoint> briskPoints;
            fast->detect(gray, fastPoints);
            brisk->detect(gray, briskPoints);
            cv::Mat fastImage;
            cv::Mat briskImage;
            cv::drawKeypoints(color, fastPoints, fastImage, cv::Scalar(0, 0, 255));
            cv::drawKeypoints(color, briskPoints, briskImage, cv::Scalar(0, 255, 0));
            showTriplet("Lesson48 FAST", fastImage, "Lesson48 BRISK", briskImage, "Lesson48 Source", color);
            statusMessage = "FAST 更偏检测速度，BRISK 则进一步兼顾尺度和描述子思想，适合对比“检测快”和“更完整特征”的差别。";
        }
        else
        {
            auto orb = cv::ORB::create(400);
            std::vector<cv::KeyPoint> orbPoints;
            cv::Mat descriptors;
            orb->detectAndCompute(gray, cv::Mat(), orbPoints, descriptors);
            cv::Mat orbImage;
            cv::drawKeypoints(color, orbPoints, orbImage, cv::Scalar(255, 0, 0));
            showTriplet("Lesson48 Source", color, "Lesson48 ORB Keypoints", orbImage, "Lesson48 Gray", gray);
            statusMessage = "ORB 把 FAST 关键点和旋转不变描述子结合在一起，是 OpenCV 里最常见的免费特征方案之一。";
        }
        return true;
    }
    case 49:
    {
        auto orb = cv::ORB::create(500);
        cv::Mat warped;
        cv::Mat affine = cv::getRotationMatrix2D(cv::Point2f(color.cols / 2.0f, color.rows / 2.0f), 8.0, 0.95);
        cv::warpAffine(color, warped, affine, color.size());
        std::vector<cv::KeyPoint> keypoints1;
        std::vector<cv::KeyPoint> keypoints2;
        cv::Mat descriptors1;
        cv::Mat descriptors2;
        orb->detectAndCompute(color, cv::Mat(), keypoints1, descriptors1);
        orb->detectAndCompute(warped, cv::Mat(), keypoints2, descriptors2);
        if (demoId == 0)
        {
            cv::BFMatcher matcher(cv::NORM_HAMMING, true);
            std::vector<cv::DMatch> matches;
            matcher.match(descriptors1, descriptors2, matches);
            std::sort(matches.begin(), matches.end(), [](const cv::DMatch &lhs, const cv::DMatch &rhs) {
                return lhs.distance < rhs.distance;
            });
            if (matches.size() > 40)
            {
                matches.resize(40);
            }
            cv::Mat draw;
            cv::drawMatches(color, keypoints1, warped, keypoints2, matches, draw);
            showTriplet("Lesson49 Source", color, "Lesson49 Warped", warped, "Lesson49 BFMatcher", draw);
            statusMessage = "暴力匹配会把每个描述子和另一张图里的所有候选都比一遍，最直接但不等于最鲁棒。";
        }
        else
        {
            cv::BFMatcher matcher(cv::NORM_HAMMING);
            std::vector<std::vector<cv::DMatch>> knnMatches;
            matcher.knnMatch(descriptors1, descriptors2, knnMatches, 2);
            std::vector<cv::DMatch> goodMatches;
            for (const auto &pair : knnMatches)
            {
                if (pair.size() == 2 && pair[0].distance < 0.75f * pair[1].distance)
                {
                    goodMatches.push_back(pair[0]);
                }
            }
            cv::Mat draw;
            cv::drawMatches(color, keypoints1, warped, keypoints2, goodMatches, draw);
            showTriplet("Lesson49 Source", color, "Lesson49 Warped", warped, "Lesson49 Ratio Test", draw);
            statusMessage = "KNN 加比值测试的目标是过滤“最优匹配和次优匹配差不多”的不稳定对应，这是传统匹配里最重要的去噪步骤之一。";
        }
        return true;
    }
    case 50:
    {
        std::vector<cv::Mat> frames = buildSyntheticVideoSequence();
        if (demoId == 0)
        {
            const std::string outputPath = saveSyntheticVideo(frames);
            showTriplet("Lesson50 Frame0", frames[0], "Lesson50 Frame8", frames[8], "Lesson50 Frame16", frames[16]);
            statusMessage = outputPath.empty()
                ? "VideoWriter 打开失败，当前环境可能缺少对应编码器。"
                : (std::string("已写出示例视频：") + outputPath + "。视频处理本质上就是把单帧算法放进时间循环。");
        }
        else
        {
            const std::string outputPath = saveSyntheticVideo(frames);
            cv::VideoCapture capture(outputPath);
            if (!capture.isOpened())
            {
                statusMessage = "无法重新读取写出的视频文件，可能是编码器或容器格式不可用。";
                return false;
            }
            cv::Mat frameA;
            cv::Mat frameB;
            cv::Mat frameC;
            capture >> frameA;
            for (int i = 0; i < 10 && capture.isOpened(); ++i)
            {
                capture >> frameB;
            }
            for (int i = 0; i < 10 && capture.isOpened(); ++i)
            {
                capture >> frameC;
            }
            showTriplet("Lesson50 Read A", frameA, "Lesson50 Read B", frameB, "Lesson50 Read C", frameC);
            statusMessage = "VideoCapture 读取的是连续帧流，读取失败、帧为空、帧率和编码格式都是工程中必须处理的问题。";
        }
        return true;
    }
    case 51:
    {
        std::vector<cv::Mat> frames = buildSyntheticVideoSequence();
        if (demoId == 0)
        {
            auto subtractor = cv::createBackgroundSubtractorMOG2();
            cv::Mat mask;
            for (const auto &frame : frames)
            {
                subtractor->apply(frame, mask);
            }
            showTriplet("Lesson51 Last Frame", frames.back(), "Lesson51 MOG2 Mask", mask, "Lesson51 Mask Color", ensureColor(mask));
            statusMessage = "背景建模的本质是持续估计“这个像素平时长什么样”，然后把偏离该模型的区域提出来。";
        }
        else
        {
            auto subtractor = cv::createBackgroundSubtractorKNN();
            cv::Mat mask;
            for (const auto &frame : frames)
            {
                subtractor->apply(frame, mask);
            }
            showTriplet("Lesson51 Last Frame", frames.back(), "Lesson51 KNN Mask", mask, "Lesson51 Mask Color", ensureColor(mask));
            statusMessage = "KNN 背景模型是另一类常见方法。它和 MOG2 在动态背景、阴影和更新速度上的表现会有所不同。";
        }
        return true;
    }
    case 52:
    {
        std::vector<cv::Mat> frames = buildSyntheticVideoSequence();
        cv::Mat grayA = toGray(frames[6]);
        cv::Mat grayB = toGray(frames[7]);
        cv::Mat diff;
        cv::absdiff(grayA, grayB, diff);
        cv::Mat binary;
        cv::threshold(diff, binary, 25, 255, cv::THRESH_BINARY);
        if (demoId == 0)
        {
            showTriplet("Lesson52 Frame A", grayA, "Lesson52 Frame B", grayB, "Lesson52 Diff", binary);
            statusMessage = "帧间差分用最直接的方式把“有变化的地方”提出来，因此很适合教学入门和简单场景的快速检测。";
        }
        else
        {
            cv::Mat preview = frames[7].clone();
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            for (const auto &contour : contours)
            {
                if (cv::contourArea(contour) > 50.0)
                {
                    cv::rectangle(preview, cv::boundingRect(contour), cv::Scalar(0, 0, 255), 2);
                }
            }
            showTriplet("Lesson52 Diff", diff, "Lesson52 Binary", binary, "Lesson52 Motion Boxes", preview);
            statusMessage = "帧差通常还要配合阈值、形态学和轮廓筛选，才能变成真正稳定的运动目标框。";
        }
        return true;
    }
    case 53:
    {
        std::vector<cv::Mat> frames = buildSyntheticVideoSequence();
        cv::Mat prevGray = toGray(frames[5]);
        cv::Mat nextGray = toGray(frames[6]);
        if (demoId == 0)
        {
            std::vector<cv::Point2f> prevPoints;
            cv::goodFeaturesToTrack(prevGray, prevPoints, 80, 0.01, 8.0);
            std::vector<cv::Point2f> nextPoints;
            std::vector<uchar> status;
            std::vector<float> errors;
            cv::calcOpticalFlowPyrLK(prevGray, nextGray, prevPoints, nextPoints, status, errors);
            cv::Mat preview = frames[6].clone();
            for (std::size_t i = 0; i < prevPoints.size(); ++i)
            {
                if (!status[i])
                {
                    continue;
                }
                cv::line(preview, prevPoints[i], nextPoints[i], cv::Scalar(0, 255, 0), 2);
                cv::circle(preview, nextPoints[i], 3, cv::Scalar(0, 0, 255), -1);
            }
            showTriplet("Lesson53 Prev", frames[5], "Lesson53 Next", frames[6], "Lesson53 LK Flow", preview);
            statusMessage = "稀疏光流跟踪的是“少量好点”的运动轨迹，适合角点跟踪和局部运动分析。";
        }
        else
        {
            cv::Mat flow;
            cv::calcOpticalFlowFarneback(prevGray, nextGray, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
            cv::Mat flowPreview = drawFlowField(nextGray, flow);
            showTriplet("Lesson53 Prev", frames[5], "Lesson53 Next", frames[6], "Lesson53 Farneback", flowPreview);
            statusMessage = "稠密光流尝试估计每个像素的运动向量，因此能形成完整运动场，但计算量也更高。";
        }
        return true;
    }
    case 54:
    {
        std::vector<cv::Mat> frames = buildSyntheticVideoSequence();
        cv::Rect trackWindow(55, 145, 70, 70);
        cv::Mat hsv;
        cv::cvtColor(frames[0], hsv, cv::COLOR_BGR2HSV);
        cv::Mat roi = hsv(trackWindow);
        int histSize = 16;
        float range[] = {0, 180};
        const float *ranges = {range};
        int channels[] = {0};
        cv::Mat hist;
        cv::calcHist(&roi, 1, channels, cv::Mat(), hist, 1, &histSize, &ranges);
        cv::normalize(hist, hist, 0, 255, cv::NORM_MINMAX);

        cv::Mat preview = frames[0].clone();
        cv::rectangle(preview, trackWindow, cv::Scalar(0, 0, 255), 2);
        cv::Mat lastFrame = frames.back().clone();
        for (std::size_t i = 1; i < frames.size(); ++i)
        {
            cv::Mat currentHsv;
            cv::cvtColor(frames[i], currentHsv, cv::COLOR_BGR2HSV);
            cv::Mat backProject;
            cv::calcBackProject(&currentHsv, 1, channels, hist, backProject, &ranges);
            if (demoId == 0)
            {
                cv::meanShift(backProject, trackWindow, cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1));
                cv::rectangle(lastFrame, trackWindow, cv::Scalar(0, 255, 0), 2);
            }
            else
            {
                cv::RotatedRect box = cv::CamShift(backProject, trackWindow, cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1));
                cv::Point2f points[4];
                box.points(points);
                for (int j = 0; j < 4; ++j)
                {
                    cv::line(lastFrame, points[j], points[(j + 1) % 4], cv::Scalar(0, 255, 255), 2);
                }
            }
        }
        showTriplet("Lesson54 Init Window", preview, demoId == 0 ? "Lesson54 MeanShift" : "Lesson54 CamShift", lastFrame, "Lesson54 Last Frame", frames.back());
        statusMessage = (demoId == 0)
            ? "MeanShift 会在概率图上不断移动窗口，找到局部峰值中心，但窗口大小本身不自适应。"
            : "CamShift 在 MeanShift 基础上还能调整窗口大小和方向，因此更适合目标尺寸变化明显的情况。";
        return true;
    }
    case 55:
    {
        auto orb = cv::ORB::create(500);
        cv::Mat warped;
        std::vector<cv::Point2f> src{{80, 60}, {static_cast<float>(color.cols - 80), 50}, {static_cast<float>(color.cols - 50), static_cast<float>(color.rows - 60)}, {70, static_cast<float>(color.rows - 40)}};
        std::vector<cv::Point2f> dst{{120, 80}, {static_cast<float>(color.cols - 100), 40}, {static_cast<float>(color.cols - 30), static_cast<float>(color.rows - 80)}, {100, static_cast<float>(color.rows - 20)}};
        cv::Mat H = cv::getPerspectiveTransform(src, dst);
        cv::warpPerspective(color, warped, H, color.size());
        std::vector<cv::KeyPoint> kp1;
        std::vector<cv::KeyPoint> kp2;
        cv::Mat desc1;
        cv::Mat desc2;
        orb->detectAndCompute(color, cv::Mat(), kp1, desc1);
        orb->detectAndCompute(warped, cv::Mat(), kp2, desc2);
        cv::BFMatcher matcher(cv::NORM_HAMMING);
        std::vector<std::vector<cv::DMatch>> knn;
        matcher.knnMatch(desc1, desc2, knn, 2);
        std::vector<cv::DMatch> good;
        for (const auto &pair : knn)
        {
            if (pair.size() == 2 && pair[0].distance < 0.75f * pair[1].distance)
            {
                good.push_back(pair[0]);
            }
        }
        std::vector<cv::Point2f> pts1;
        std::vector<cv::Point2f> pts2;
        for (const auto &match : good)
        {
            pts1.push_back(kp1[match.queryIdx].pt);
            pts2.push_back(kp2[match.trainIdx].pt);
        }
        cv::Mat inlierMask;
        cv::Mat homography = cv::findHomography(pts2, pts1, cv::RANSAC, 3.0, inlierMask);
        if (demoId == 0)
        {
            std::vector<cv::DMatch> inliers;
            for (int i = 0; i < inlierMask.rows; ++i)
            {
                if (inlierMask.at<uchar>(i, 0) != 0)
                {
                    inliers.push_back(good[static_cast<std::size_t>(i)]);
                }
            }
            cv::Mat draw;
            cv::drawMatches(color, kp1, warped, kp2, inliers, draw);
            showTriplet("Lesson55 Reference", color, "Lesson55 Warped", warped, "Lesson55 Inliers", draw);
            statusMessage = "RANSAC 的作用不是“让匹配更漂亮”，而是让单应矩阵估计不会被少量误匹配拖偏。";
        }
        else
        {
            cv::Mat aligned;
            cv::warpPerspective(warped, aligned, homography, color.size());
            cv::Mat diff;
            cv::absdiff(color, aligned, diff);
            showTriplet("Lesson55 Reference", color, "Lesson55 Aligned", aligned, "Lesson55 Difference", diff);
            statusMessage = "单应性把一个近似平面的视角变化统一成 3x3 映射，因此是拼接和配准的关键工具。";
        }
        return true;
    }
    case 56:
    {
        cv::Mat chessboard = makeChessboardImage();
        const cv::Size patternSize(7, 5);
        std::vector<cv::Point2f> corners;
        const bool found = cv::findChessboardCorners(toGray(chessboard), patternSize, corners);
        cv::Mat preview = chessboard.clone();
        if (found)
        {
            cv::drawChessboardCorners(preview, patternSize, corners, found);
        }
        if (demoId == 0)
        {
            showTriplet("Lesson56 Chessboard", chessboard, "Lesson56 Corners", preview, "Lesson56 Gray", toGray(chessboard));
            statusMessage = "标定首先依赖稳定的角点检测。棋盘格规则、角点清晰，因此成为最经典的标定板。";
        }
        else
        {
            cv::Mat textPreview(320, 640, CV_8UC3, cv::Scalar(245, 245, 245));
            cv::putText(textPreview, "object points: (x, y, 0)", cv::Point(30, 80), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(30, 30, 30), 2);
            cv::putText(textPreview, "image points: detected corners", cv::Point(30, 130), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(30, 30, 30), 2);
            cv::putText(textPreview, "calibrateCamera(object, image, ...)", cv::Point(30, 210), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 180), 2);
            showTriplet("Lesson56 Chessboard", chessboard, "Lesson56 Corners", preview, "Lesson56 Calibration Template", textPreview);
            statusMessage = "标定的本质是把“已知世界点”与“检测到的图像点”对应起来，从而求出相机内参和畸变参数。";
        }
        return true;
    }
    case 57:
    {
        cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) << 550.0, 0.0, color.cols / 2.0, 0.0, 550.0, color.rows / 2.0, 0.0, 0.0, 1.0);
        cv::Mat distCoeffs = (cv::Mat_<double>(1, 5) << -0.22, 0.08, 0.0, 0.0, 0.0);
        if (demoId == 0)
        {
            cv::Mat undistorted;
            cv::undistort(color, undistorted, cameraMatrix, distCoeffs);
            showTriplet("Lesson57 Source", color, "Lesson57 Undistort", undistorted, "Lesson57 Gray", toGray(undistorted));
            statusMessage = "undistort 是最直接的畸变校正入口，前提是你已经有可信的相机矩阵和畸变参数。";
        }
        else
        {
            cv::Mat map1;
            cv::Mat map2;
            cv::initUndistortRectifyMap(cameraMatrix, distCoeffs, cv::Mat(), cameraMatrix, color.size(), CV_16SC2, map1, map2);
            cv::Mat remapped;
            cv::remap(color, remapped, map1, map2, cv::INTER_LINEAR);
            showTriplet("Lesson57 Source", color, "Lesson57 Remap", remapped, "Lesson57 Gray", toGray(remapped));
            statusMessage = "remap 方式更适合在视频流里重复使用，因为映射表可以预先计算后反复复用。";
        }
        return true;
    }
    case 58:
    {
        cv::Mat left(240, 400, CV_8UC1, cv::Scalar(40));
        cv::Mat right = left.clone();
        cv::rectangle(left, cv::Rect(120, 70, 80, 80), cv::Scalar(200), cv::FILLED);
        cv::rectangle(left, cv::Rect(240, 100, 50, 90), cv::Scalar(160), cv::FILLED);
        cv::rectangle(right, cv::Rect(104, 70, 80, 80), cv::Scalar(200), cv::FILLED);
        cv::rectangle(right, cv::Rect(228, 100, 50, 90), cv::Scalar(160), cv::FILLED);
        auto stereo = cv::StereoBM::create(32, 15);
        cv::Mat disparity;
        stereo->compute(left, right, disparity);
        cv::Mat disparity8 = normalizeToU8(disparity);
        if (demoId == 0)
        {
            showTriplet("Lesson58 Left", left, "Lesson58 Right", right, "Lesson58 Disparity", disparity8);
            statusMessage = "左右图中同一点的位置偏移就是视差。一般视差越大，目标越靠近相机。";
        }
        else
        {
            cv::Mat heat;
            cv::applyColorMap(disparity8, heat, cv::COLORMAP_JET);
            showTriplet("Lesson58 Left", left, "Lesson58 Disparity", disparity8, "Lesson58 Heatmap", heat);
            statusMessage = "视差图常用伪彩色显示，以便更直观看出前后层次。真实双目任务还需要先做极线校正。";
        }
        return true;
    }
    case 59:
    {
        if (demoId == 0)
        {
            cv::Mat blob = cv::dnn::blobFromImage(color, 1.0 / 255.0, cv::Size(224, 224), cv::Scalar(), true, false);
            std::vector<cv::Mat> blobImages;
            cv::dnn::imagesFromBlob(blob, blobImages);
            cv::Mat blobVis(260, 520, CV_8UC3, cv::Scalar(245, 245, 245));
            cv::putText(blobVis, "blob dims = " + std::to_string(blob.dims), cv::Point(30, 70), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(30, 30, 30), 2);
            cv::putText(blobVis, "shape = [" + std::to_string(blob.size[0]) + "," + std::to_string(blob.size[1]) + "," + std::to_string(blob.size[2]) + "," + std::to_string(blob.size[3]) + "]", cv::Point(30, 120), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(30, 30, 30), 2);
            showTriplet("Lesson59 Source", color, "Lesson59 Resized", blobImages.front(), "Lesson59 Blob Info", blobVis);
            statusMessage = "DNN 推理的第一步不是“跑模型”，而是把输入图像整理成网络真正接受的张量格式。";
        }
        else
        {
            cv::Mat preview = color.clone();
            std::vector<cv::Rect> boxes{{80, 60, 140, 120}, {95, 75, 140, 120}, {300, 140, 160, 130}};
            std::vector<float> scores{0.82f, 0.74f, 0.91f};
            std::vector<int> kept;
            cv::dnn::NMSBoxes(boxes, scores, 0.5f, 0.4f, kept);
            for (int index : kept)
            {
                cv::rectangle(preview, boxes[static_cast<std::size_t>(index)], cv::Scalar(0, 255, 0), 2);
                cv::putText(preview, cv::format("score=%.2f", scores[static_cast<std::size_t>(index)]), boxes[static_cast<std::size_t>(index)].tl() + cv::Point(0, -6), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
            }
            showTriplet("Lesson59 Source", color, "Lesson59 Candidate Boxes", preview, "Lesson59 Gray", toGray(preview));
            statusMessage = "真实检测模型的后处理通常包括置信度筛选、NMS 去重和类别解析，这部分同样决定最终视觉结果。";
        }
        return true;
    }
    case 60:
    {
        if (demoId == 0)
        {
            cv::TickMeter totalFrame;
            cv::TickMeter roiFrame;
            cv::Mat resultFull;
            cv::Mat resultRoi = color.clone();
            totalFrame.start();
            for (int i = 0; i < 80; ++i)
            {
                cv::GaussianBlur(color, resultFull, cv::Size(11, 11), 2.0);
                cv::Canny(toGray(resultFull), resultFull, 50, 150);
            }
            totalFrame.stop();

            cv::Rect roi(color.cols / 4, color.rows / 4, color.cols / 2, color.rows / 2);
            roiFrame.start();
            for (int i = 0; i < 80; ++i)
            {
                cv::Mat roiView = resultRoi(roi);
                cv::GaussianBlur(roiView, roiView, cv::Size(11, 11), 2.0);
            }
            roiFrame.stop();

            cv::Mat timing(280, 620, CV_8UC3, cv::Scalar(245, 245, 245));
            cv::putText(timing, cv::format("full pipeline: %.2f ms", totalFrame.getTimeMilli()), cv::Point(30, 90), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(30, 30, 30), 2);
            cv::putText(timing, cv::format("roi only: %.2f ms", roiFrame.getTimeMilli()), cv::Point(30, 150), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(30, 30, 30), 2);
            cv::rectangle(resultRoi, roi, cv::Scalar(0, 0, 255), 2);
            showTriplet("Lesson60 Full Result", resultFull, "Lesson60 ROI Marked", resultRoi, "Lesson60 Timing", timing);
            statusMessage = "优化首先要学会测量。很多时候最直接的提速手段不是换算法，而是减少处理范围。";
        }
        else
        {
            cv::TickMeter loadTimer;
            cv::TickMeter grayTimer;
            cv::TickMeter edgeTimer;
            loadTimer.start();
            cv::Mat local = color.clone();
            loadTimer.stop();
            grayTimer.start();
            cv::Mat gray = toGray(local);
            grayTimer.stop();
            edgeTimer.start();
            cv::Mat edges;
            cv::Canny(gray, edges, 60, 180);
            edgeTimer.stop();

            cv::Mat timing(320, 620, CV_8UC3, cv::Scalar(245, 245, 245));
            cv::putText(timing, cv::format("clone/load: %.3f ms", loadTimer.getTimeMilli()), cv::Point(30, 90), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(30, 30, 30), 2);
            cv::putText(timing, cv::format("toGray: %.3f ms", grayTimer.getTimeMilli()), cv::Point(30, 140), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(30, 30, 30), 2);
            cv::putText(timing, cv::format("Canny: %.3f ms", edgeTimer.getTimeMilli()), cv::Point(30, 190), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(30, 30, 30), 2);
            cv::putText(timing, "Measure first, optimize second.", cv::Point(30, 250), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 180), 2);
            showTriplet("Lesson60 Source", color, "Lesson60 Edges", edges, "Lesson60 TickMeter", timing);
            statusMessage = "把流水线拆成可计时的小步骤，远比凭感觉猜瓶颈更可靠。这也是视觉工程化的基本习惯。";
        }
        return true;
    }
    default:
        break;
    }

    return false;
}
} // namespace

const AdvancedLessonConfig &advancedLessonConfig(int lessonId)
{
    const auto &configMap = configs();
    auto it = configMap.find(lessonId);
    if (it == configMap.end())
    {
        return configMap.begin()->second;
    }
    return it->second;
}

bool runAdvancedLessonDemo(int lessonId, int demoId, std::string &statusMessage)
{
    const bool ok = lessonDemo(lessonId, demoId, statusMessage);
    if (!ok)
    {
        statusMessage = "该课的 Qt 演示入口尚未注册成功，请检查 lessonId 和 demoId。";
    }
    return ok;
}

void closeAdvancedLessonWindows()
{
    for (const auto &name : windowRegistry())
    {
        cv::destroyWindow(name);
    }
    windowRegistry().clear();
}