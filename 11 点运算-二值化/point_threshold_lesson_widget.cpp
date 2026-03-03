#include "point_threshold_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

PointThresholdLessonWidget::PointThresholdLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("点运算：二值化"), this);
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

    connect(openButton, &QPushButton::clicked, this, &PointThresholdLessonWidget::openAndShow);
}

void PointThresholdLessonWidget::openAndShow()
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

    const double thresholdValue = 128.0;
    cv::Mat binary;
    cv::threshold(gray, binary, thresholdValue, 255.0, cv::THRESH_BINARY);

    originalWindowName = "Original (Gray)";
    processedWindowName = "Binary";
    cv::namedWindow(originalWindowName, cv::WINDOW_NORMAL);
    cv::namedWindow(processedWindowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(originalWindowName, 432, 648);
    cv::resizeWindow(processedWindowName, 432, 648);
    cv::imshow(originalWindowName, gray);
    cv::imshow(processedWindowName, binary);

    statusLabel->setText(QStringLiteral("二值化：threshold=%1").arg(thresholdValue, 0, 'f', 0));
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}
