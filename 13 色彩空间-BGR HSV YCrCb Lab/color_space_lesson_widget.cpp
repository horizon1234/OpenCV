#include "color_space_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

ColorSpaceLessonWidget::ColorSpaceLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("色彩空间：BGR / HSV / YCrCb / Lab"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    auto *row1 = new QHBoxLayout();
    auto *btnBGR = new QPushButton(QStringLiteral("BGR 通道分离"), this);
    auto *btnHSV = new QPushButton(QStringLiteral("HSV 通道可视化"), this);
    auto *btnYCrCb = new QPushButton(QStringLiteral("YCrCb 通道可视化"), this);
    row1->addStretch();
    row1->addWidget(btnBGR);
    row1->addWidget(btnHSV);
    row1->addWidget(btnYCrCb);
    row1->addStretch();

    auto *row2 = new QHBoxLayout();
    auto *btnLab = new QPushButton(QStringLiteral("Lab 通道可视化"), this);
    auto *btnCompare = new QPushButton(QStringLiteral("用途对比：调色 / 饱和度 / 亮度"), this);
    row2->addStretch();
    row2->addWidget(btnLab);
    row2->addWidget(btnCompare);
    row2->addStretch();

    auto *row3 = new QHBoxLayout();
    auto *btnHSVSeg = new QPushButton(QStringLiteral("实战：HSV 红色提取"), this);
    auto *btnBGRWrong = new QPushButton(QStringLiteral("实战：BGR 阈值误区"), this);
    row3->addStretch();
    row3->addWidget(btnHSVSeg);
    row3->addWidget(btnBGRWrong);
    row3->addStretch();

    statusLabel = new QLabel(QStringLiteral("建议按顺序观察：BGR -> HSV -> YCrCb -> Lab -> 用途对比"), this);
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

    connect(btnBGR, &QPushButton::clicked, this, &ColorSpaceLessonWidget::showBGRChannels);
    connect(btnHSV, &QPushButton::clicked, this, &ColorSpaceLessonWidget::showHSVChannels);
    connect(btnYCrCb, &QPushButton::clicked, this, &ColorSpaceLessonWidget::showYCrCbChannels);
    connect(btnLab, &QPushButton::clicked, this, &ColorSpaceLessonWidget::showLabChannels);
    connect(btnCompare, &QPushButton::clicked, this, &ColorSpaceLessonWidget::showUseCaseComparison);
    connect(btnHSVSeg, &QPushButton::clicked, this, &ColorSpaceLessonWidget::showHSVRedSegmentation);
    connect(btnBGRWrong, &QPushButton::clicked, this, &ColorSpaceLessonWidget::showBGRTresholdPitfall);
}

void ColorSpaceLessonWidget::ensureTimer()
{
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}

void ColorSpaceLessonWidget::closeAllWindows()
{
    for (const auto &name : windowNames)
    {
        cv::destroyWindow(name);
    }
    windowNames.clear();
}

cv::Mat ColorSpaceLessonWidget::loadColorImage() const
{
    return cv::imread("cat.jpg", cv::IMREAD_COLOR);
}

cv::Mat ColorSpaceLessonWidget::makeGrayPreview(const cv::Mat &channel)
{
    cv::Mat preview;
    cv::cvtColor(channel, preview, cv::COLOR_GRAY2BGR);
    return preview;
}

cv::Mat ColorSpaceLessonWidget::makeHuePreview(const cv::Mat &hChannel)
{
    cv::Mat saturation(hChannel.size(), CV_8UC1, cv::Scalar(255));
    cv::Mat value(hChannel.size(), CV_8UC1, cv::Scalar(255));

    std::vector<cv::Mat> hsvChannels = {hChannel, saturation, value};
    cv::Mat hsvPreview;
    cv::merge(hsvChannels, hsvPreview);

    cv::Mat bgrPreview;
    cv::cvtColor(hsvPreview, bgrPreview, cv::COLOR_HSV2BGR);
    return bgrPreview;
}

cv::Mat ColorSpaceLessonWidget::makeMaskPreview(const cv::Mat &mask)
{
    cv::Mat preview;
    cv::cvtColor(mask, preview, cv::COLOR_GRAY2BGR);
    return preview;
}

void ColorSpaceLessonWidget::showBGRChannels()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    std::vector<cv::Mat> bgrChannels;
    cv::split(color, bgrChannels);

    const std::string winOrig = "Original (BGR)";
    const std::string winBlue = "B Channel";
    const std::string winGreen = "G Channel";
    const std::string winRed = "R Channel";
    windowNames = {winOrig, winBlue, winGreen, winRed};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winBlue, cv::WINDOW_NORMAL);
    cv::namedWindow(winGreen, cv::WINDOW_NORMAL);
    cv::namedWindow(winRed, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winBlue, 432, 648);
    cv::resizeWindow(winGreen, 432, 648);
    cv::resizeWindow(winRed, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winBlue, makeGrayPreview(bgrChannels[0]));
    cv::imshow(winGreen, makeGrayPreview(bgrChannels[1]));
    cv::imshow(winRed, makeGrayPreview(bgrChannels[2]));

    statusLabel->setText(QStringLiteral("BGR 是 OpenCV 默认空间。三个窗口显示的是三个原始颜色分量的强弱，不是“亮度图”。"));
    ensureTimer();
}

void ColorSpaceLessonWidget::showHSVChannels()
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

    std::vector<cv::Mat> hsvChannels;
    cv::split(hsv, hsvChannels);

    const std::string winOrig = "Original";
    const std::string winHue = "HSV - H (Hue Preview)";
    const std::string winSat = "HSV - S (Saturation)";
    const std::string winVal = "HSV - V (Value)";
    windowNames = {winOrig, winHue, winSat, winVal};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winHue, cv::WINDOW_NORMAL);
    cv::namedWindow(winSat, cv::WINDOW_NORMAL);
    cv::namedWindow(winVal, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winHue, 432, 648);
    cv::resizeWindow(winSat, 432, 648);
    cv::resizeWindow(winVal, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winHue, makeHuePreview(hsvChannels[0]));
    cv::imshow(winSat, makeGrayPreview(hsvChannels[1]));
    cv::imshow(winVal, makeGrayPreview(hsvChannels[2]));

    statusLabel->setText(QStringLiteral("HSV 更适合表达颜色语义。H 表示颜色类别，S 表示鲜艳程度，V 表示明度。注意 OpenCV 中 H 范围是 0~179。"));
    ensureTimer();
}

void ColorSpaceLessonWidget::showYCrCbChannels()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat ycrcb;
    cv::cvtColor(color, ycrcb, cv::COLOR_BGR2YCrCb);

    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);

    const std::string winOrig = "Original";
    const std::string winY = "YCrCb - Y (Luma)";
    const std::string winCr = "YCrCb - Cr";
    const std::string winCb = "YCrCb - Cb";
    windowNames = {winOrig, winY, winCr, winCb};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winY, cv::WINDOW_NORMAL);
    cv::namedWindow(winCr, cv::WINDOW_NORMAL);
    cv::namedWindow(winCb, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winY, 432, 648);
    cv::resizeWindow(winCr, 432, 648);
    cv::resizeWindow(winCb, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winY, makeGrayPreview(channels[0]));
    cv::imshow(winCr, makeGrayPreview(channels[1]));
    cv::imshow(winCb, makeGrayPreview(channels[2]));

    statusLabel->setText(QStringLiteral("YCrCb 把亮度和色度分开。Y 是亮度，Cr/Cb 是色差，不是原始红蓝通道，所以它很适合“只改亮度”。"));
    ensureTimer();
}

void ColorSpaceLessonWidget::showLabChannels()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat lab;
    cv::cvtColor(color, lab, cv::COLOR_BGR2Lab);

    std::vector<cv::Mat> channels;
    cv::split(lab, channels);

    const std::string winOrig = "Original";
    const std::string winL = "Lab - L";
    const std::string winA = "Lab - a";
    const std::string winB = "Lab - b";
    windowNames = {winOrig, winL, winA, winB};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winL, cv::WINDOW_NORMAL);
    cv::namedWindow(winA, cv::WINDOW_NORMAL);
    cv::namedWindow(winB, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winL, 432, 648);
    cv::resizeWindow(winA, 432, 648);
    cv::resizeWindow(winB, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winL, makeGrayPreview(channels[0]));
    cv::imshow(winA, makeGrayPreview(channels[1]));
    cv::imshow(winB, makeGrayPreview(channels[2]));

    statusLabel->setText(QStringLiteral("Lab 更接近感知空间。L 是明度，a 表示绿到红，b 表示蓝到黄。a、b 在 OpenCV 8 位表示中通常以 128 附近为中性。"));
    ensureTimer();
}

void ColorSpaceLessonWidget::showUseCaseComparison()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat bgrWarm = color.clone();
    {
        std::vector<cv::Mat> channels;
        cv::split(bgrWarm, channels);
        channels[2].convertTo(channels[2], -1, 1.15, 0.0);
        channels[0].convertTo(channels[0], -1, 0.85, 0.0);
        cv::merge(channels, bgrWarm);
    }

    cv::Mat hsvBoost;
    {
        cv::Mat hsv;
        cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);
        std::vector<cv::Mat> channels;
        cv::split(hsv, channels);
        channels[1].convertTo(channels[1], -1, 1.35, 0.0);
        cv::merge(channels, hsv);
        cv::cvtColor(hsv, hsvBoost, cv::COLOR_HSV2BGR);
    }

    cv::Mat yBoost;
    {
        cv::Mat ycrcb;
        cv::cvtColor(color, ycrcb, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> channels;
        cv::split(ycrcb, channels);
        cv::add(channels[0], cv::Scalar(25), channels[0]);
        cv::merge(channels, ycrcb);
        cv::cvtColor(ycrcb, yBoost, cv::COLOR_YCrCb2BGR);
    }

    cv::Mat lBoost;
    {
        cv::Mat lab;
        cv::cvtColor(color, lab, cv::COLOR_BGR2Lab);
        std::vector<cv::Mat> channels;
        cv::split(lab, channels);
        cv::add(channels[0], cv::Scalar(20), channels[0]);
        cv::merge(channels, lab);
        cv::cvtColor(lab, lBoost, cv::COLOR_Lab2BGR);
    }

    const std::string winOrig = "Original";
    const std::string winBGR = "BGR Warm Tone";
    const std::string winHSV = "HSV Saturation Boost";
    const std::string winY = "YCrCb Y Boost";
    const std::string winL = "Lab L Boost";
    windowNames = {winOrig, winBGR, winHSV, winY, winL};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winBGR, cv::WINDOW_NORMAL);
    cv::namedWindow(winHSV, cv::WINDOW_NORMAL);
    cv::namedWindow(winY, cv::WINDOW_NORMAL);
    cv::namedWindow(winL, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winBGR, 432, 648);
    cv::resizeWindow(winHSV, 432, 648);
    cv::resizeWindow(winY, 432, 648);
    cv::resizeWindow(winL, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winBGR, bgrWarm);
    cv::imshow(winHSV, hsvBoost);
    cv::imshow(winY, yBoost);
    cv::imshow(winL, lBoost);

    statusLabel->setText(QStringLiteral("用途对比：BGR 适合直接通道调色；HSV 适合调饱和度；YCrCb 和 Lab 更适合只提亮明暗层次。"));
    ensureTimer();
}

void ColorSpaceLessonWidget::showHSVRedSegmentation()
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
    cv::inRange(hsv, cv::Scalar(0, 80, 50), cv::Scalar(12, 255, 255), mask1);
    cv::inRange(hsv, cv::Scalar(168, 80, 50), cv::Scalar(179, 255, 255), mask2);

    cv::Mat mask;
    cv::bitwise_or(mask1, mask2, mask);

    cv::Mat extracted;
    cv::bitwise_and(color, color, extracted, mask);

    const std::string winOrig = "Original";
    const std::string winMask1 = "HSV Red Mask (Range 1)";
    const std::string winMask2 = "HSV Red Mask (Range 2)";
    const std::string winResult = "HSV Red Extraction";
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
    cv::imshow(winMask1, makeMaskPreview(mask1));
    cv::imshow(winMask2, makeMaskPreview(mask2));
    cv::imshow(winResult, extracted);

    statusLabel->setText(QStringLiteral("HSV 分割实战：红色在 H 环上跨越首尾，所以通常要用两段区间再做 OR 合并。"));
    ensureTimer();
}

void ColorSpaceLessonWidget::showBGRTresholdPitfall()
{
    closeAllWindows();

    const cv::Mat color = loadColorImage();
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：cat.jpg（请确认在当前目录）"));
        return;
    }

    cv::Mat wrongMask;
    cv::inRange(color, cv::Scalar(0, 0, 80), cv::Scalar(120, 120, 255), wrongMask);

    cv::Mat hsv;
    cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);
    cv::Mat correctMask1;
    cv::Mat correctMask2;
    cv::Mat correctMask;
    cv::inRange(hsv, cv::Scalar(0, 80, 50), cv::Scalar(12, 255, 255), correctMask1);
    cv::inRange(hsv, cv::Scalar(168, 80, 50), cv::Scalar(179, 255, 255), correctMask2);
    cv::bitwise_or(correctMask1, correctMask2, correctMask);

    const std::string winOrig = "Original";
    const std::string winWrong = "BGR Range Mask (Often Wrong)";
    const std::string winCorrect = "HSV Range Mask (Recommended)";
    windowNames = {winOrig, winWrong, winCorrect};

    cv::namedWindow(winOrig, cv::WINDOW_NORMAL);
    cv::namedWindow(winWrong, cv::WINDOW_NORMAL);
    cv::namedWindow(winCorrect, cv::WINDOW_NORMAL);
    cv::resizeWindow(winOrig, 432, 648);
    cv::resizeWindow(winWrong, 432, 648);
    cv::resizeWindow(winCorrect, 432, 648);
    cv::imshow(winOrig, color);
    cv::imshow(winWrong, makeMaskPreview(wrongMask));
    cv::imshow(winCorrect, makeMaskPreview(correctMask));

    statusLabel->setText(QStringLiteral("BGR 阈值误区：直接用 BGR 三通道框颜色，通常对光照和明暗变化更敏感；颜色分割更推荐 HSV。"));
    ensureTimer();
}