#include "point_histogram_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

PointHistogramLessonWidget::PointHistogramLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("点运算：直方图均衡化"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    // ── 按钮行 1：正确做法 & 错误示例 ──
    auto *row1 = new QHBoxLayout();
    auto *btnYCrCb = new QPushButton(QStringLiteral("✅ YCrCb 均衡化（正确）"), this);
    auto *btnWrongBGR = new QPushButton(QStringLiteral("❌ BGR 均衡化（错误示例）"), this);
    row1->addStretch();
    row1->addWidget(btnYCrCb);
    row1->addWidget(btnWrongBGR);
    row1->addStretch();

    // ── 按钮行 2：CLAHE & 实验 ──
    auto *row2 = new QHBoxLayout();
    auto *btnCLAHE = new QPushButton(QStringLiteral("CLAHE 自适应均衡化"), this);
    auto *btnHistVis = new QPushButton(QStringLiteral("可视化直方图"), this);
    auto *btnCompare = new QPushButton(QStringLiteral("全局 vs CLAHE 对比"), this);
    row2->addStretch();
    row2->addWidget(btnCLAHE);
    row2->addWidget(btnHistVis);
    row2->addWidget(btnCompare);
    row2->addStretch();

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));

    layout->addWidget(titleLabel);
    layout->addLayout(row1);
    layout->addLayout(row2);
    layout->addWidget(statusLabel);

    waitKeyTimer = new QTimer(this);
    waitKeyTimer->setInterval(30);
    connect(waitKeyTimer, &QTimer::timeout, this, []() {
        cv::waitKey(1);
    });

    connect(btnYCrCb, &QPushButton::clicked, this, &PointHistogramLessonWidget::showYCrCbEqualization);
    connect(btnWrongBGR, &QPushButton::clicked, this, &PointHistogramLessonWidget::showWrongBGREqualization);
    connect(btnCLAHE, &QPushButton::clicked, this, &PointHistogramLessonWidget::showCLAHE);
    connect(btnHistVis, &QPushButton::clicked, this, &PointHistogramLessonWidget::showHistogramVisualization);
    connect(btnCompare, &QPushButton::clicked, this, &PointHistogramLessonWidget::showGlobalVsCLAHE);
}

// ──────────────────────────────────────────────
// 辅助方法
// ──────────────────────────────────────────────

void PointHistogramLessonWidget::ensureTimer()
{
    if (!waitKeyTimer->isActive())
        waitKeyTimer->start();
}

void PointHistogramLessonWidget::closeAllWindows()
{
    for (const auto &name : windowNames)
        cv::destroyWindow(name);
    windowNames.clear();
}

cv::Mat PointHistogramLessonWidget::drawHistogram(const cv::Mat &gray, const cv::Scalar &color)
{
    // 计算直方图
    int histSize = 256;
    float range[] = {0, 256};
    const float *histRange = {range};
    cv::Mat hist;
    cv::calcHist(&gray, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

    // 画出直方图
    const int histW = 512;
    const int histH = 400;
    cv::Mat histImage(histH, histW, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX);

    const int binW = cvRound(static_cast<double>(histW) / histSize);
    for (int i = 1; i < histSize; i++)
    {
        cv::line(histImage,
                 cv::Point(binW * (i - 1), histH - cvRound(hist.at<float>(i - 1))),
                 cv::Point(binW * i, histH - cvRound(hist.at<float>(i))),
                 color, 2);
    }
    return histImage;
}

// ──────────────────────────────────────────────
// 演示 1：YCrCb 均衡化（正确做法）
// ──────────────────────────────────────────────

void PointHistogramLessonWidget::showYCrCbEqualization()
{
    closeAllWindows();

    const cv::Mat color = cv::imread("cat.jpg", cv::IMREAD_COLOR);
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    // BGR → YCrCb → 只均衡化 Y 通道 → YCrCb → BGR
    cv::Mat ycrcb;
    cv::cvtColor(color, ycrcb, cv::COLOR_BGR2YCrCb);

    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);
    cv::equalizeHist(channels[0], channels[0]);
    cv::merge(channels, ycrcb);

    cv::Mat equalized;
    cv::cvtColor(ycrcb, equalized, cv::COLOR_YCrCb2BGR);

    const std::string winOrig = "Original";
    const std::string winProc = "YCrCb Equalized (Correct)";
    windowNames = {winOrig, winProc};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winProc, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winProc, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winProc, equalized);

    statusLabel->setText(QStringLiteral("✅ 正确：在 YCrCb 空间只均衡化 Y 通道，颜色不失真"));
    ensureTimer();
}

// ──────────────────────────────────────────────
// 演示 2：错误示例 —— 直接对 BGR 三通道分别均衡化
// ──────────────────────────────────────────────

void PointHistogramLessonWidget::showWrongBGREqualization()
{
    closeAllWindows();

    const cv::Mat color = cv::imread("cat.jpg", cv::IMREAD_COLOR);
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    // ❌ 错误做法：分别对 B、G、R 三通道各自做均衡化
    // 三通道的拉伸比例不同，导致 BGR 之间的比例被打破 → 颜色严重失真
    std::vector<cv::Mat> bgr;
    cv::split(color, bgr);
    cv::equalizeHist(bgr[0], bgr[0]); // B 通道均衡
    cv::equalizeHist(bgr[1], bgr[1]); // G 通道均衡
    cv::equalizeHist(bgr[2], bgr[2]); // R 通道均衡
    cv::Mat wrongResult;
    cv::merge(bgr, wrongResult);

    // ✅ 正确做法（对比用）
    cv::Mat ycrcb;
    cv::cvtColor(color, ycrcb, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);
    cv::equalizeHist(channels[0], channels[0]);
    cv::merge(channels, ycrcb);
    cv::Mat correctResult;
    cv::cvtColor(ycrcb, correctResult, cv::COLOR_YCrCb2BGR);

    const std::string winOrig = "Original";
    const std::string winWrong = "WRONG: BGR Equalized (Color Distorted!)";
    const std::string winCorrect = "CORRECT: YCrCb Equalized";
    windowNames = {winOrig, winWrong, winCorrect};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winWrong, cv::WINDOW_NORMAL);
    cv::namedWindow(winCorrect, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winWrong, 432, 648);
    cv::resizeWindow(winCorrect, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winWrong, wrongResult);
    cv::imshow(winCorrect, correctResult);

    statusLabel->setText(QStringLiteral("❌ BGR 直接均衡化：颜色严重失真 | ✅ YCrCb：颜色正常"));
    ensureTimer();
}

// ──────────────────────────────────────────────
// 演示 3：CLAHE 自适应直方图均衡化
// ──────────────────────────────────────────────

void PointHistogramLessonWidget::showCLAHE()
{
    closeAllWindows();

    const cv::Mat color = cv::imread("cat.jpg", cv::IMREAD_COLOR);
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    // 在 YCrCb 空间做 CLAHE，只处理 Y 通道
    cv::Mat ycrcb;
    cv::cvtColor(color, ycrcb, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);

    // 创建 CLAHE 对象：clipLimit=2.0, tileSize=8×8
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(channels[0], channels[0]);

    cv::merge(channels, ycrcb);
    cv::Mat claheResult;
    cv::cvtColor(ycrcb, claheResult, cv::COLOR_YCrCb2BGR);

    const std::string winOrig = "Original";
    const std::string winCLAHE = "CLAHE (clipLimit=2.0, tile=8x8)";
    windowNames = {winOrig, winCLAHE};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winCLAHE, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winCLAHE, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winCLAHE, claheResult);

    statusLabel->setText(QStringLiteral("CLAHE：分块自适应均衡化，局部增强不过度，适合明暗不均的图片"));
    ensureTimer();
}

// ──────────────────────────────────────────────
// 实验 1：可视化直方图（均衡化前后）
// ──────────────────────────────────────────────

void PointHistogramLessonWidget::showHistogramVisualization()
{
    closeAllWindows();

    const cv::Mat color = cv::imread("cat.jpg", cv::IMREAD_COLOR);
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    // 转灰度
    cv::Mat gray;
    cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);

    // 均衡化
    cv::Mat equalized;
    cv::equalizeHist(gray, equalized);

    // 画出均衡化前后的直方图
    cv::Mat histBefore = drawHistogram(gray, cv::Scalar(255, 0, 0));       // 蓝色线
    cv::Mat histAfter = drawHistogram(equalized, cv::Scalar(0, 0, 255));   // 红色线

    const std::string winGray = "Grayscale Original";
    const std::string winEq = "Grayscale Equalized";
    const std::string winHistBefore = "Histogram BEFORE Equalization";
    const std::string winHistAfter = "Histogram AFTER Equalization";
    windowNames = {winGray, winEq, winHistBefore, winHistAfter};

    cv::namedWindow(winGray, cv::WINDOW_NORMAL);
    cv::namedWindow(winEq, cv::WINDOW_NORMAL);
    cv::namedWindow(winHistBefore, cv::WINDOW_NORMAL);
    cv::namedWindow(winHistAfter, cv::WINDOW_NORMAL);
    cv::resizeWindow(winGray, 432, 648);
    cv::resizeWindow(winEq, 432, 648);
    cv::resizeWindow(winHistBefore, 512, 400);
    cv::resizeWindow(winHistAfter, 512, 400);
    cv::imshow(winGray, gray);
    cv::imshow(winEq, equalized);
    cv::imshow(winHistBefore, histBefore);
    cv::imshow(winHistAfter, histAfter);

    statusLabel->setText(QStringLiteral("直方图可视化：对比均衡化前后的灰度分布变化"));
    ensureTimer();
}

// ──────────────────────────────────────────────
// 实验 2：全局均衡化 vs CLAHE 对比
// ──────────────────────────────────────────────

void PointHistogramLessonWidget::showGlobalVsCLAHE()
{
    closeAllWindows();

    const cv::Mat color = cv::imread("cat.jpg", cv::IMREAD_COLOR);
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    // 转灰度
    cv::Mat gray;
    cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);

    // 全局均衡化
    cv::Mat globalEq;
    cv::equalizeHist(gray, globalEq);

    // CLAHE
    cv::Mat claheEq;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(gray, claheEq);

    // 画出各自的直方图
    cv::Mat histOrig = drawHistogram(gray, cv::Scalar(128, 128, 128));
    cv::Mat histGlobal = drawHistogram(globalEq, cv::Scalar(0, 0, 255));
    cv::Mat histCLAHE = drawHistogram(claheEq, cv::Scalar(0, 128, 0));

    const std::string winOrig = "Original (Gray)";
    const std::string winGlobal = "Global equalizeHist";
    const std::string winCLAHE = "CLAHE (clipLimit=2.0)";
    const std::string winHOrig = "Hist: Original";
    const std::string winHGlobal = "Hist: Global Eq";
    const std::string winHCLAHE = "Hist: CLAHE";
    windowNames = {winOrig, winGlobal, winCLAHE, winHOrig, winHGlobal, winHCLAHE};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winGlobal, cv::WINDOW_NORMAL);
    cv::namedWindow(winCLAHE, cv::WINDOW_NORMAL);
    cv::namedWindow(winHOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winHGlobal, cv::WINDOW_NORMAL);
    cv::namedWindow(winHCLAHE, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winGlobal, 432, 648);
    cv::resizeWindow(winCLAHE, 432, 648);
    cv::resizeWindow(winHOrig, 512, 400);
    cv::resizeWindow(winHGlobal, 512, 400);
    cv::resizeWindow(winHCLAHE, 512, 400);
    cv::imshow(winOrig, gray);
    cv::imshow(winGlobal, globalEq);
    cv::imshow(winCLAHE, claheEq);
    cv::imshow(winHOrig, histOrig);
    cv::imshow(winHGlobal, histGlobal);
    cv::imshow(winHCLAHE, histCLAHE);

    statusLabel->setText(QStringLiteral("全局均衡化 vs CLAHE：观察 CLAHE 局部增强更温和，不会过度放大噪声"));
    ensureTimer();
}
