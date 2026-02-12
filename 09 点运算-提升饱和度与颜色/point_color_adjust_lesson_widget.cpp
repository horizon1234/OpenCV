#include "point_color_adjust_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

PointColorAdjustLessonWidget::PointColorAdjustLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("点运算：提升饱和度 / 颜色增强抑制"), this);
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

    connect(openButton, &QPushButton::clicked, this, &PointColorAdjustLessonWidget::openAndShow);
}

void PointColorAdjustLessonWidget::openAndShow()
{
    const QString imagePath = QStringLiteral("cat.jpg");
    const cv::Mat color = cv::imread(imagePath.toStdString(), cv::IMREAD_COLOR);
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：%1（请确认在当前目录）").arg(imagePath));
        return;
    }

    cv::Mat hsv;
    cv::cvtColor(color, hsv, cv::COLOR_BGR2HSV);

    std::vector<cv::Mat> hsvChannels;
    cv::split(hsv, hsvChannels);
    cv::add(hsvChannels[1], cv::Scalar(40), hsvChannels[1]);
    cv::merge(hsvChannels, hsv);

    cv::Mat saturated;
    cv::cvtColor(hsv, saturated, cv::COLOR_HSV2BGR);

    std::vector<cv::Mat> bgrChannels;
    cv::split(saturated, bgrChannels);
    bgrChannels[2].convertTo(bgrChannels[2], -1, 1.2, 0.0);
    bgrChannels[0].convertTo(bgrChannels[0], -1, 0.8, 0.0);
    cv::merge(bgrChannels, saturated);

    originalWindowName = "Original";
    processedWindowName = "Saturation & Color";
    cv::namedWindow(originalWindowName, cv::WINDOW_NORMAL);
    cv::namedWindow(processedWindowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(originalWindowName, 432, 648);
    cv::resizeWindow(processedWindowName, 432, 648);
    cv::imshow(originalWindowName, color);
    cv::imshow(processedWindowName, saturated);

    statusLabel->setText(QStringLiteral("S 通道 +40，红色增强 1.2 倍，蓝色抑制 0.8 倍"));
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}
