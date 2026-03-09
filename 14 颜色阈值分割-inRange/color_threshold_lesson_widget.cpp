#include "color_threshold_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

namespace
{
struct HsvTrackbarState
{
    cv::Mat original;
    cv::Mat hsv;
    std::string sourceWindow;
    std::string maskWindow;
    std::string resultWindow;
    int hMin = 0;
    int hMax = 179;
    int sMin = 80;
    int sMax = 255;
    int vMin = 50;
    int vMax = 255;
};

HsvTrackbarState *gHsvTrackbarState = nullptr;

void renderHsvTrackbarState(HsvTrackbarState *state)
{
    if (!state || state->original.empty() || state->hsv.empty())
    {
        return;
    }

    const int hMin = std::min(state->hMin, state->hMax);
    const int hMax = std::max(state->hMin, state->hMax);
    const int sMin = std::min(state->sMin, state->sMax);
    const int sMax = std::max(state->sMin, state->sMax);
    const int vMin = std::min(state->vMin, state->vMax);
    const int vMax = std::max(state->vMin, state->vMax);

    cv::Mat mask;
    cv::inRange(state->hsv,
                cv::Scalar(hMin, sMin, vMin),
                cv::Scalar(hMax, sMax, vMax),
                mask);

    cv::Mat result;
    cv::bitwise_and(state->original, state->original, result, mask);

    cv::imshow(state->sourceWindow, state->original);
    cv::imshow(state->maskWindow, mask);
    cv::imshow(state->resultWindow, result);
}

void onHsvTrackbar(int, void *)
{
    renderHsvTrackbarState(gHsvTrackbarState);
}
} // namespace

ColorThresholdLessonWidget::ColorThresholdLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("颜色阈值分割：inRange"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    auto *row1 = new QHBoxLayout();
    auto *btnSingle = new QPushButton(QStringLiteral("HSV 单区间分割"), this);
    auto *btnRed = new QPushButton(QStringLiteral("红色双区间分割"), this);
    row1->addStretch();
    row1->addWidget(btnSingle);
    row1->addWidget(btnRed);
    row1->addStretch();

    auto *row2 = new QHBoxLayout();
    auto *btnCleanup = new QPushButton(QStringLiteral("Mask 清理：开闭运算"), this);
    auto *btnCompare = new QPushButton(QStringLiteral("BGR vs HSV 对比"), this);
    row2->addStretch();
    row2->addWidget(btnCleanup);
    row2->addWidget(btnCompare);
    row2->addStretch();

    auto *row3 = new QHBoxLayout();
    auto *btnTrackbar = new QPushButton(QStringLiteral("HSV 阈值实时调节"), this);
    row3->addStretch();
    row3->addWidget(btnTrackbar);
    row3->addStretch();

    statusLabel = new QLabel(QStringLiteral("建议顺序：先看单区间，再看红色双区间，最后看 mask 清理和 BGR/HSV 对比。"), this);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));
    statusLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addLayout(row1);
    layout->addLayout(row2);
    layout->addLayout(row3);
    layout->addWidget(statusLabel);

    waitKeyTimer = new QTimer(this);
    waitKeyTimer->setInterval(30);
    connect(waitKeyTimer, &QTimer::timeout, this, []() {
        cv::waitKey(1);
    });

    connect(btnSingle, &QPushButton::clicked, this, &ColorThresholdLessonWidget::showSingleHSVRange);
    connect(btnRed, &QPushButton::clicked, this, &ColorThresholdLessonWidget::showRedDualRange);
    connect(btnCleanup, &QPushButton::clicked, this, &ColorThresholdLessonWidget::showMaskCleanup);
    connect(btnCompare, &QPushButton::clicked, this, &ColorThresholdLessonWidget::showBGRvsHSV);
    connect(btnTrackbar, &QPushButton::clicked, this, &ColorThresholdLessonWidget::showHSVTrackbarTuning);
}

void ColorThresholdLessonWidget::ensureTimer()
{
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}

void ColorThresholdLessonWidget::closeAllWindows()
{
    for (const auto &name : windowNames)
    {
        cv::destroyWindow(name);
    }
    windowNames.clear();
}

cv::Mat ColorThresholdLessonWidget::loadColorImage() const
{
    return cv::imread("cat.jpg", cv::IMREAD_COLOR);
}

cv::Mat ColorThresholdLessonWidget::maskToBgr(const cv::Mat &mask)
{
    cv::Mat preview;
    cv::cvtColor(mask, preview, cv::COLOR_GRAY2BGR);
    return preview;
}

void ColorThresholdLessonWidget::showSingleHSVRange()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat hsv;
    cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    cv::inRange(hsv, cv::Scalar(35, 60, 40), cv::Scalar(95, 255, 255), mask);

    cv::Mat result;
    cv::bitwise_and(color, color, result, mask);

    const std::string winOrig = "Original";
    const std::string winMask = "HSV Single Range Mask";
    const std::string winResult = "HSV Single Range Result";
    windowNames = {winOrig, winMask, winResult};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winMask, cv::WINDOW_NORMAL);
    cv::namedWindow(winResult, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winMask, 432, 648);
    cv::resizeWindow(winResult, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winMask, maskToBgr(mask));
    cv::imshow(winResult, result);

    statusLabel->setText(QStringLiteral("单区间示例：H 决定颜色类别，S/V 给下限过滤灰色和暗部噪声。"));
    ensureTimer();
}

void ColorThresholdLessonWidget::showRedDualRange()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat hsv;
    cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask1;
    cv::Mat mask2;
    cv::Mat mask;
    cv::inRange(hsv, cv::Scalar(0, 80, 50), cv::Scalar(12, 255, 255), mask1);
    cv::inRange(hsv, cv::Scalar(168, 80, 50), cv::Scalar(179, 255, 255), mask2);
    cv::bitwise_or(mask1, mask2, mask);

    cv::Mat result;
    cv::bitwise_and(color, color, result, mask);

    const std::string winOrig = "Original";
    const std::string winMask1 = "Red Mask 1";
    const std::string winMask2 = "Red Mask 2";
    const std::string winResult = "Red Dual Range Result";
    windowNames = {winOrig, winMask1, winMask2, winResult};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winMask1, cv::WINDOW_NORMAL);
    cv::namedWindow(winMask2, cv::WINDOW_NORMAL);
    cv::namedWindow(winResult, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winMask1, 432, 648);
    cv::resizeWindow(winMask2, 432, 648);
    cv::resizeWindow(winResult, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winMask1, maskToBgr(mask1));
    cv::imshow(winMask2, maskToBgr(mask2));
    cv::imshow(winResult, result);

    statusLabel->setText(QStringLiteral("红色双区间：因为 H 是环形，所以红色常常要 split 成两段再 OR 合并。"));
    ensureTimer();
}

void ColorThresholdLessonWidget::showMaskCleanup()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat hsv;
    cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);

    cv::Mat rawMask;
    cv::inRange(hsv, cv::Scalar(0, 60, 40), cv::Scalar(20, 255, 255), rawMask);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::Mat cleanMask;
    cv::morphologyEx(rawMask, cleanMask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(cleanMask, cleanMask, cv::MORPH_CLOSE, kernel);

    cv::Mat rawResult;
    cv::Mat cleanResult;
    cv::bitwise_and(color, color, rawResult, rawMask);
    cv::bitwise_and(color, color, cleanResult, cleanMask);

    const std::string winRawMask = "Raw Mask";
    const std::string winCleanMask = "Clean Mask (Open + Close)";
    const std::string winRawResult = "Raw Extraction";
    const std::string winCleanResult = "Clean Extraction";
    windowNames = {winRawMask, winCleanMask, winRawResult, winCleanResult};

    cv::namedWindow(winRawMask, cv::WINDOW_NORMAL);
    cv::namedWindow(winCleanMask, cv::WINDOW_NORMAL);
    cv::namedWindow(winRawResult, cv::WINDOW_NORMAL);
    cv::namedWindow(winCleanResult, cv::WINDOW_NORMAL);
    cv::resizeWindow(winRawMask, 432, 648);
    cv::resizeWindow(winCleanMask, 432, 648);
    cv::resizeWindow(winRawResult, 432, 648);
    cv::resizeWindow(winCleanResult, 432, 648);
    cv::imshow(winRawMask, maskToBgr(rawMask));
    cv::imshow(winCleanMask, maskToBgr(cleanMask));
    cv::imshow(winRawResult, rawResult);
    cv::imshow(winCleanResult, cleanResult);

    statusLabel->setText(QStringLiteral("Mask 清理：开运算先去小白点，闭运算再补小黑洞。真实项目里这一步通常很必要。"));
    ensureTimer();
}

void ColorThresholdLessonWidget::showBGRvsHSV()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat bgrMask;
    cv::inRange(color, cv::Scalar(0, 0, 80), cv::Scalar(120, 120, 255), bgrMask);

    cv::Mat hsv;
    cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);
    cv::Mat hsvMask1;
    cv::Mat hsvMask2;
    cv::Mat hsvMask;
    cv::inRange(hsv, cv::Scalar(0, 80, 50), cv::Scalar(12, 255, 255), hsvMask1);
    cv::inRange(hsv, cv::Scalar(168, 80, 50), cv::Scalar(179, 255, 255), hsvMask2);
    cv::bitwise_or(hsvMask1, hsvMask2, hsvMask);

    const std::string winOrig = "Original";
    const std::string winBGR = "BGR inRange Mask";
    const std::string winHSV = "HSV inRange Mask";
    windowNames = {winOrig, winBGR, winHSV};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winBGR, cv::WINDOW_NORMAL);
    cv::namedWindow(winHSV, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winBGR, 432, 648);
    cv::resizeWindow(winHSV, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winBGR, maskToBgr(bgrMask));
    cv::imshow(winHSV, maskToBgr(hsvMask));

    statusLabel->setText(QStringLiteral("BGR vs HSV：同样是颜色提取，HSV 通常更贴近颜色语义，也更容易通过 S/V 下限抑制噪声。"));
    ensureTimer();
}

void ColorThresholdLessonWidget::showHSVTrackbarTuning()
{
    closeAllWindows();

    static HsvTrackbarState state;
    gHsvTrackbarState = &state;

    state.original = loadColorImage();
    if (state.original.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::cvtColor(state.original, state.hsv, cv::COLOR_BGR2HSV);
    state.sourceWindow = "HSV Trackbar - Original";
    state.maskWindow = "HSV Trackbar - Mask";
    state.resultWindow = "HSV Trackbar - Result";

    windowNames = {state.sourceWindow, state.maskWindow, state.resultWindow};

    cv::namedWindow(state.sourceWindow, cv::WINDOW_NORMAL);
    cv::namedWindow(state.maskWindow, cv::WINDOW_NORMAL);
    cv::namedWindow(state.resultWindow, cv::WINDOW_NORMAL);
    cv::resizeWindow(state.sourceWindow, 432, 648);
    cv::resizeWindow(state.maskWindow, 432, 648);
    cv::resizeWindow(state.resultWindow, 432, 648);

    cv::createTrackbar("H Min", state.maskWindow, &state.hMin, 179, onHsvTrackbar, nullptr);
    cv::createTrackbar("H Max", state.maskWindow, &state.hMax, 179, onHsvTrackbar, nullptr);
    cv::createTrackbar("S Min", state.maskWindow, &state.sMin, 255, onHsvTrackbar, nullptr);
    cv::createTrackbar("S Max", state.maskWindow, &state.sMax, 255, onHsvTrackbar, nullptr);
    cv::createTrackbar("V Min", state.maskWindow, &state.vMin, 255, onHsvTrackbar, nullptr);
    cv::createTrackbar("V Max", state.maskWindow, &state.vMax, 255, onHsvTrackbar, nullptr);

    renderHsvTrackbarState(&state);

    statusLabel->setText(QStringLiteral("实时调参：拖动 H/S/V 上下限观察 mask 变化。这是实际做 inRange 最常见的调参方式。"));
    ensureTimer();
}