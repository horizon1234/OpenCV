#include "point_truncation_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

PointTruncationLessonWidget::PointTruncationLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("点运算：截断"), this);
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

    connect(openButton, &QPushButton::clicked, this, &PointTruncationLessonWidget::openAndShow);
}

void PointTruncationLessonWidget::openAndShow()
{
    const QString imagePath = QStringLiteral("cat.jpg");
    const cv::Mat color = cv::imread(imagePath.toStdString(), cv::IMREAD_COLOR);
    if (color.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：%1（请确认在当前目录）").arg(imagePath));
        return;
    }

    cv::Mat gray;
    cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);

    const double thresholdValue = 120.0;
    cv::Mat truncated;
    cv::threshold(gray, truncated, thresholdValue, 255.0, cv::THRESH_TRUNC);

    originalWindowName = "Original (Gray)";
    processedWindowName = "Truncated";
    cv::namedWindow(originalWindowName, cv::WINDOW_NORMAL);
    cv::namedWindow(processedWindowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(originalWindowName, 432, 648);
    cv::resizeWindow(processedWindowName, 432, 648);
    cv::imshow(originalWindowName, gray);
    cv::imshow(processedWindowName, truncated);

    statusLabel->setText(QStringLiteral("阈值截断：threshold=%1").arg(thresholdValue, 0, 'f', 0));
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}
