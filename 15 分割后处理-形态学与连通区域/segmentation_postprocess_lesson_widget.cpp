#include "segmentation_postprocess_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

namespace
{
struct MorphologyKernelState
{
    cv::Mat rawMask;
    std::string rawWindow;
    std::string openWindow;
    std::string closeWindow;
    int kernelRadius = 2;
};

MorphologyKernelState *gMorphologyKernelState = nullptr;

void renderMorphologyKernelState(MorphologyKernelState *state)
{
    if (!state || state->rawMask.empty())
    {
        return;
    }

    const int kernelSize = std::max(1, state->kernelRadius * 2 + 1);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));

    cv::Mat opened;
    cv::Mat closed;
    cv::morphologyEx(state->rawMask, opened, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(opened, closed, cv::MORPH_CLOSE, kernel);

    cv::imshow(state->rawWindow, state->rawMask);
    cv::imshow(state->openWindow, opened);
    cv::imshow(state->closeWindow, closed);
}

void onMorphologyKernelTrackbar(int, void *)
{
    renderMorphologyKernelState(gMorphologyKernelState);
}
} // namespace

SegmentationPostprocessLessonWidget::SegmentationPostprocessLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("分割后处理：形态学与连通区域"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    auto *row1 = new QHBoxLayout();
    auto *btnOpenClose = new QPushButton(QStringLiteral("Raw vs Open/Close"), this);
    auto *btnKernelTrackbar = new QPushButton(QStringLiteral("Kernel 实时调节"), this);
    auto *btnGradient = new QPushButton(QStringLiteral("形态学梯度"), this);
    row1->addStretch();
    row1->addWidget(btnOpenClose);
    row1->addWidget(btnKernelTrackbar);
    row1->addWidget(btnGradient);
    row1->addStretch();

    auto *row2 = new QHBoxLayout();
    auto *btnLargest = new QPushButton(QStringLiteral("最大连通区域"), this);
    auto *btnPipeline = new QPushButton(QStringLiteral("完整后处理流水线"), this);
    row2->addStretch();
    row2->addWidget(btnLargest);
    row2->addWidget(btnPipeline);
    row2->addStretch();

    statusLabel = new QLabel(QStringLiteral("建议顺序：先看 Open/Close，再看最大连通区域，最后看完整流水线。"), this);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));
    statusLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addLayout(row1);
    layout->addLayout(row2);
    layout->addWidget(statusLabel);

    waitKeyTimer = new QTimer(this);
    waitKeyTimer->setInterval(30);
    connect(waitKeyTimer, &QTimer::timeout, this, []() {
        cv::waitKey(1);
    });

    connect(btnOpenClose, &QPushButton::clicked, this, &SegmentationPostprocessLessonWidget::showRawVsOpenClose);
    connect(btnKernelTrackbar, &QPushButton::clicked, this, &SegmentationPostprocessLessonWidget::showKernelTrackbarTuning);
    connect(btnGradient, &QPushButton::clicked, this, &SegmentationPostprocessLessonWidget::showMorphologyGradient);
    connect(btnLargest, &QPushButton::clicked, this, &SegmentationPostprocessLessonWidget::showLargestComponent);
    connect(btnPipeline, &QPushButton::clicked, this, &SegmentationPostprocessLessonWidget::showFullPipeline);
}

void SegmentationPostprocessLessonWidget::ensureTimer()
{
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}

void SegmentationPostprocessLessonWidget::closeAllWindows()
{
    for (const auto &name : windowNames)
    {
        cv::destroyWindow(name);
    }
    windowNames.clear();
}

cv::Mat SegmentationPostprocessLessonWidget::loadColorImage() const
{
    return cv::imread("cat.jpg", cv::IMREAD_COLOR);
}

cv::Mat SegmentationPostprocessLessonWidget::buildRawMask(const cv::Mat &color) const
{
    cv::Mat hsv;
    cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask1;
    cv::Mat mask2;
    cv::Mat rawMask;
    cv::inRange(hsv, cv::Scalar(0, 60, 40), cv::Scalar(20, 255, 255), mask1);
    cv::inRange(hsv, cv::Scalar(160, 60, 40), cv::Scalar(179, 255, 255), mask2);
    cv::bitwise_or(mask1, mask2, rawMask);
    return rawMask;
}

cv::Mat SegmentationPostprocessLessonWidget::maskToBgr(const cv::Mat &mask)
{
    cv::Mat preview;
    cv::cvtColor(mask, preview, cv::COLOR_GRAY2BGR);
    return preview;
}

void SegmentationPostprocessLessonWidget::showRawVsOpenClose()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat rawMask = buildRawMask(color);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));

    cv::Mat opened;
    cv::Mat closed;
    cv::morphologyEx(rawMask, opened, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(opened, closed, cv::MORPH_CLOSE, kernel);

    const std::string winRaw = "Raw Mask";
    const std::string winOpen = "After Open";
    const std::string winClose = "After Open + Close";
    windowNames = {winRaw, winOpen, winClose};

    cv::namedWindow(winRaw, cv::WINDOW_NORMAL);
    cv::namedWindow(winOpen, cv::WINDOW_NORMAL);
    cv::namedWindow(winClose, cv::WINDOW_NORMAL);
    cv::resizeWindow(winRaw, 432, 648);
    cv::resizeWindow(winOpen, 432, 648);
    cv::resizeWindow(winClose, 432, 648);
    cv::imshow(winRaw, maskToBgr(rawMask));
    cv::imshow(winOpen, maskToBgr(opened));
    cv::imshow(winClose, maskToBgr(closed));

    statusLabel->setText(QStringLiteral("Open 先去小白点，Close 再补小黑洞。它们处理的是 mask 的“形状质量”。"));
    ensureTimer();
}

void SegmentationPostprocessLessonWidget::showKernelTrackbarTuning()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    static MorphologyKernelState state;
    gMorphologyKernelState = &state;

    state.rawMask = buildRawMask(color);
    state.rawWindow = "Morphology Trackbar - Raw Mask";
    state.openWindow = "Morphology Trackbar - Open";
    state.closeWindow = "Morphology Trackbar - Open + Close";
    state.kernelRadius = 2;

    windowNames = {state.rawWindow, state.openWindow, state.closeWindow};

    cv::namedWindow(state.rawWindow, cv::WINDOW_NORMAL);
    cv::namedWindow(state.openWindow, cv::WINDOW_NORMAL);
    cv::namedWindow(state.closeWindow, cv::WINDOW_NORMAL);
    cv::resizeWindow(state.rawWindow, 432, 648);
    cv::resizeWindow(state.openWindow, 432, 648);
    cv::resizeWindow(state.closeWindow, 432, 648);

    cv::createTrackbar("Kernel Radius", state.closeWindow, &state.kernelRadius, 10, onMorphologyKernelTrackbar, nullptr);
    renderMorphologyKernelState(&state);

    statusLabel->setText(QStringLiteral("实时调 kernel：观察核从 3x3、5x5、7x7 变大后，open/close 对 mask 的影响会如何加强。"));
    ensureTimer();
}

void SegmentationPostprocessLessonWidget::showMorphologyGradient()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat rawMask = buildRawMask(color);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::Mat gradient;
    cv::morphologyEx(rawMask, gradient, cv::MORPH_GRADIENT, kernel);

    const std::string winRaw = "Raw Mask";
    const std::string winGradient = "Morphology Gradient";
    windowNames = {winRaw, winGradient};

    cv::namedWindow(winRaw, cv::WINDOW_NORMAL);
    cv::namedWindow(winGradient, cv::WINDOW_NORMAL);
    cv::resizeWindow(winRaw, 432, 648);
    cv::resizeWindow(winGradient, 432, 648);
    cv::imshow(winRaw, maskToBgr(rawMask));
    cv::imshow(winGradient, maskToBgr(gradient));

    statusLabel->setText(QStringLiteral("形态学梯度更像边界观察工具：它突出的是轮廓，而不是去噪或填洞。"));
    ensureTimer();
}

void SegmentationPostprocessLessonWidget::showLargestComponent()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat rawMask = buildRawMask(color);
    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;
    const int numLabels = cv::connectedComponentsWithStats(rawMask, labels, stats, centroids);

    int bestLabel = 0;
    int bestArea = 0;
    for (int label = 1; label < numLabels; ++label)
    {
        const int area = stats.at<int>(label, cv::CC_STAT_AREA);
        if (area > bestArea)
        {
            bestArea = area;
            bestLabel = label;
        }
    }

    cv::Mat largestMask = cv::Mat::zeros(rawMask.size(), CV_8UC1);
    if (bestLabel > 0)
    {
        largestMask.setTo(255, labels == bestLabel);
    }

    cv::Mat result;
    cv::bitwise_and(color, color, result, largestMask);

    const std::string winRaw = "Raw Mask";
    const std::string winLargest = "Largest Component Mask";
    const std::string winResult = "Largest Component Result";
    windowNames = {winRaw, winLargest, winResult};

    cv::namedWindow(winRaw, cv::WINDOW_NORMAL);
    cv::namedWindow(winLargest, cv::WINDOW_NORMAL);
    cv::namedWindow(winResult, cv::WINDOW_NORMAL);
    cv::resizeWindow(winRaw, 432, 648);
    cv::resizeWindow(winLargest, 432, 648);
    cv::resizeWindow(winResult, 432, 648);
    cv::imshow(winRaw, maskToBgr(rawMask));
    cv::imshow(winLargest, maskToBgr(largestMask));
    cv::imshow(winResult, result);

    statusLabel->setText(QStringLiteral("最大连通区域：当你只关心一个主体目标时，面积筛选通常比继续调形态学更直接。"));
    ensureTimer();
}

void SegmentationPostprocessLessonWidget::showFullPipeline()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat rawMask = buildRawMask(color);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::Mat cleanMask;
    cv::morphologyEx(rawMask, cleanMask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(cleanMask, cleanMask, cv::MORPH_CLOSE, kernel);

    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;
    const int numLabels = cv::connectedComponentsWithStats(cleanMask, labels, stats, centroids);

    int bestLabel = 0;
    int bestArea = 0;
    for (int label = 1; label < numLabels; ++label)
    {
        const int area = stats.at<int>(label, cv::CC_STAT_AREA);
        if (area > bestArea)
        {
            bestArea = area;
            bestLabel = label;
        }
    }

    cv::Mat finalMask = cv::Mat::zeros(cleanMask.size(), CV_8UC1);
    if (bestLabel > 0)
    {
        finalMask.setTo(255, labels == bestLabel);
    }

    cv::Mat result;
    cv::bitwise_and(color, color, result, finalMask);

    const std::string winRaw = "Pipeline - Raw Mask";
    const std::string winClean = "Pipeline - Clean Mask";
    const std::string winFinal = "Pipeline - Final Mask";
    const std::string winResult = "Pipeline - Final Result";
    windowNames = {winRaw, winClean, winFinal, winResult};

    cv::namedWindow(winRaw, cv::WINDOW_NORMAL);
    cv::namedWindow(winClean, cv::WINDOW_NORMAL);
    cv::namedWindow(winFinal, cv::WINDOW_NORMAL);
    cv::namedWindow(winResult, cv::WINDOW_NORMAL);
    cv::resizeWindow(winRaw, 432, 648);
    cv::resizeWindow(winClean, 432, 648);
    cv::resizeWindow(winFinal, 432, 648);
    cv::resizeWindow(winResult, 432, 648);
    cv::imshow(winRaw, maskToBgr(rawMask));
    cv::imshow(winClean, maskToBgr(cleanMask));
    cv::imshow(winFinal, maskToBgr(finalMask));
    cv::imshow(winResult, result);

    statusLabel->setText(QStringLiteral("完整流水线：raw mask -> open -> close -> largest component。这个顺序在很多单目标分割任务里很实用。"));
    ensureTimer();
}