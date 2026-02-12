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

    auto *buttonLayout = new QHBoxLayout();
    auto *openButton = new QPushButton(QStringLiteral("打开并显示"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(openButton);
    buttonLayout->addStretch();

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));

    layout->addWidget(titleLabel);
    layout->addLayout(buttonLayout);
    layout->addWidget(statusLabel);

    waitKeyTimer = new QTimer(this);
    waitKeyTimer->setInterval(30);
    connect(waitKeyTimer, &QTimer::timeout, this, []() {
        cv::waitKey(1);
    });

    connect(openButton, &QPushButton::clicked, this, &PointHistogramLessonWidget::openAndShow);
}

void PointHistogramLessonWidget::openAndShow()
{
    const QString imagePath = QStringLiteral("cat.jpg");
    const cv::Mat color = cv::imread(imagePath.toStdString(), cv::IMREAD_COLOR);
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：%1（请确认在当前目录）").arg(imagePath));
        return;
    }

    cv::Mat ycrcb;
    cv::cvtColor(color, ycrcb, cv::COLOR_BGR2YCrCb);

    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);
    cv::equalizeHist(channels[0], channels[0]);
    cv::merge(channels, ycrcb);

    cv::Mat equalized;
    cv::cvtColor(ycrcb, equalized, cv::COLOR_YCrCb2BGR);

    originalWindowName = "Original";
    processedWindowName = "Histogram Equalized";
    cv::namedWindow(originalWindowName, cv::WINDOW_NORMAL);
    cv::namedWindow(processedWindowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(originalWindowName, 432, 648);
    cv::resizeWindow(processedWindowName, 432, 648);
    cv::imshow(originalWindowName, color);
    cv::imshow(processedWindowName, equalized);

    statusLabel->setText(QStringLiteral("在 Y 通道做直方图均衡化"));
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}
