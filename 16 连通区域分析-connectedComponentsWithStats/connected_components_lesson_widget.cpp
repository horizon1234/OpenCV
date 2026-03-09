#include "connected_components_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

ConnectedComponentsLessonWidget::ConnectedComponentsLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("连通区域分析：connectedComponentsWithStats"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    auto *row1 = new QHBoxLayout();
    auto *btnLabels = new QPushButton(QStringLiteral("标签着色图"), this);
    auto *btnArea = new QPushButton(QStringLiteral("面积筛选"), this);
    row1->addStretch();
    row1->addWidget(btnLabels);
    row1->addWidget(btnArea);
    row1->addStretch();

    auto *row2 = new QHBoxLayout();
    auto *btnLargest = new QPushButton(QStringLiteral("最大连通区域"), this);
    auto *btnOverlay = new QPushButton(QStringLiteral("外接框与面积"), this);
    row2->addStretch();
    row2->addWidget(btnLargest);
    row2->addWidget(btnOverlay);
    row2->addStretch();

    statusLabel = new QLabel(QStringLiteral("建议顺序：先看标签着色图，再看面积筛选和最大连通区域。"), this);
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

    connect(btnLabels, &QPushButton::clicked, this, &ConnectedComponentsLessonWidget::showComponentLabels);
    connect(btnArea, &QPushButton::clicked, this, &ConnectedComponentsLessonWidget::showAreaFiltering);
    connect(btnLargest, &QPushButton::clicked, this, &ConnectedComponentsLessonWidget::showLargestComponent);
    connect(btnOverlay, &QPushButton::clicked, this, &ConnectedComponentsLessonWidget::showStatsOverlay);
}

void ConnectedComponentsLessonWidget::ensureTimer()
{
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}

void ConnectedComponentsLessonWidget::closeAllWindows()
{
    for (const auto &name : windowNames)
    {
        cv::destroyWindow(name);
    }
    windowNames.clear();
}

cv::Mat ConnectedComponentsLessonWidget::loadColorImage() const
{
    return cv::imread("cat.jpg", cv::IMREAD_COLOR);
}

cv::Mat ConnectedComponentsLessonWidget::buildRawMask(const cv::Mat &color) const
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

cv::Mat ConnectedComponentsLessonWidget::maskToBgr(const cv::Mat &mask)
{
    cv::Mat preview;
    cv::cvtColor(mask, preview, cv::COLOR_GRAY2BGR);
    return preview;
}

cv::Mat ConnectedComponentsLessonWidget::colorizeLabels(const cv::Mat &labels, int numLabels)
{
    cv::Mat colored(labels.size(), CV_8UC3, cv::Scalar(0, 0, 0));
    for (int label = 1; label < numLabels; ++label)
    {
        const cv::Vec3b color(
            static_cast<uchar>((label * 53) % 255),
            static_cast<uchar>((label * 97) % 255),
            static_cast<uchar>((label * 193) % 255));
        colored.setTo(color, labels == label);
    }
    return colored;
}

void ConnectedComponentsLessonWidget::showComponentLabels()
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
    cv::Mat labelPreview = colorizeLabels(labels, numLabels);

    const std::string winMask = "Components - Raw Mask";
    const std::string winLabels = "Components - Label Colors";
    windowNames = {winMask, winLabels};

    cv::namedWindow(winMask, cv::WINDOW_NORMAL);
    cv::namedWindow(winLabels, cv::WINDOW_NORMAL);
    cv::resizeWindow(winMask, 432, 648);
    cv::resizeWindow(winLabels, 432, 648);
    cv::imshow(winMask, maskToBgr(rawMask));
    cv::imshow(winLabels, labelPreview);

    statusLabel->setText(QStringLiteral("标签着色图：每一种颜色都代表一个独立连通区域。先看它，最容易理解 connected components 到底分了几个块。"));
    ensureTimer();
}

void ConnectedComponentsLessonWidget::showAreaFiltering()
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

    constexpr int minArea = 600;
    cv::Mat filteredMask = cv::Mat::zeros(rawMask.size(), CV_8UC1);
    for (int label = 1; label < numLabels; ++label)
    {
        const int area = stats.at<int>(label, cv::CC_STAT_AREA);
        if (area >= minArea)
        {
            filteredMask.setTo(255, labels == label);
        }
    }

    const std::string winRaw = "Area Filter - Raw Mask";
    const std::string winFiltered = "Area Filter - Min Area";
    windowNames = {winRaw, winFiltered};

    cv::namedWindow(winRaw, cv::WINDOW_NORMAL);
    cv::namedWindow(winFiltered, cv::WINDOW_NORMAL);
    cv::resizeWindow(winRaw, 432, 648);
    cv::resizeWindow(winFiltered, 432, 648);
    cv::imshow(winRaw, maskToBgr(rawMask));
    cv::imshow(winFiltered, maskToBgr(filteredMask));

    statusLabel->setText(QStringLiteral("面积筛选：删除小于 600 像素的区域。这个思路比一味继续加大形态学 kernel 更可控。"));
    ensureTimer();
}

void ConnectedComponentsLessonWidget::showLargestComponent()
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

    const std::string winRaw = "Largest Component - Raw Mask";
    const std::string winLargest = "Largest Component - Mask";
    const std::string winResult = "Largest Component - Result";
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

    statusLabel->setText(QStringLiteral("最大连通区域：适合只有一个主体目标的场景，但你仍然要警惕背景误检块恰好更大。"));
    ensureTimer();
}

void ConnectedComponentsLessonWidget::showStatsOverlay()
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

    cv::Mat overlay = color.clone();
    for (int label = 1; label < numLabels; ++label)
    {
        const int left = stats.at<int>(label, cv::CC_STAT_LEFT);
        const int top = stats.at<int>(label, cv::CC_STAT_TOP);
        const int width = stats.at<int>(label, cv::CC_STAT_WIDTH);
        const int height = stats.at<int>(label, cv::CC_STAT_HEIGHT);
        const int area = stats.at<int>(label, cv::CC_STAT_AREA);

        cv::rectangle(overlay, cv::Rect(left, top, width, height), cv::Scalar(0, 255, 255), 2);
        cv::putText(overlay,
                    cv::format("id=%d area=%d", label, area),
                    cv::Point(left, std::max(20, top - 8)),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.55,
                    cv::Scalar(0, 255, 255),
                    1,
                    cv::LINE_AA);
    }

    const std::string winMask = "Stats Overlay - Raw Mask";
    const std::string winOverlay = "Stats Overlay - Bounding Boxes";
    windowNames = {winMask, winOverlay};

    cv::namedWindow(winMask, cv::WINDOW_NORMAL);
    cv::namedWindow(winOverlay, cv::WINDOW_NORMAL);
    cv::resizeWindow(winMask, 432, 648);
    cv::resizeWindow(winOverlay, 432, 648);
    cv::imshow(winMask, maskToBgr(rawMask));
    cv::imshow(winOverlay, overlay);

    statusLabel->setText(QStringLiteral("外接框与面积：这是调试筛选规则最直观的方法，你能立刻看到每个候选块的位置和面积。"));
    ensureTimer();
}