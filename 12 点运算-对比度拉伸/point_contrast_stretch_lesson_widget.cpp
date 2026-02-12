#include "point_contrast_stretch_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

PointContrastStretchLessonWidget::PointContrastStretchLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("点运算：对比度拉伸"), this);
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

    connect(openButton, &QPushButton::clicked, this, &PointContrastStretchLessonWidget::openAndShow);
}

void PointContrastStretchLessonWidget::openAndShow()
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

    double minValue = 0.0;
    double maxValue = 0.0;
    cv::minMaxLoc(gray, &minValue, &maxValue);

    cv::Mat stretched = gray.clone();
    if (maxValue > minValue)
    {
        const double alpha = 255.0 / (maxValue - minValue);
        const double beta = -minValue * alpha;
        gray.convertTo(stretched, CV_8U, alpha, beta);
    }

    originalWindowName = "Original (Gray)";
    processedWindowName = "Contrast Stretched";
    cv::namedWindow(originalWindowName, cv::WINDOW_NORMAL);
    cv::namedWindow(processedWindowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(originalWindowName, 432, 648);
    cv::resizeWindow(processedWindowName, 432, 648);
    cv::imshow(originalWindowName, gray);
    cv::imshow(processedWindowName, stretched);

    statusLabel->setText(QStringLiteral("对比度拉伸：min=%1 max=%2")
                             .arg(minValue, 0, 'f', 1)
                             .arg(maxValue, 0, 'f', 1));
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}
