#include "point_gray_transform_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

namespace
{
cv::Mat applyGamma(const cv::Mat &gray, double gamma)
{
    cv::Mat lut(1, 256, CV_8U);
    for (int i = 0; i < 256; ++i)
    {
        const double normalized = static_cast<double>(i) / 255.0;
        const double corrected = std::pow(normalized, gamma) * 255.0;
        lut.at<uchar>(i) = cv::saturate_cast<uchar>(corrected);
    }

    cv::Mat output;
    cv::LUT(gray, lut, output);
    return output;
}
} // namespace

PointGrayTransformLessonWidget::PointGrayTransformLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("点运算：灰度变换"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    auto *buttonLayout = new QHBoxLayout();
    auto *openButton = new QPushButton(QStringLiteral("打开并显示"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(openButton);
    buttonLayout->addStretch();

    auto *sliderLayout = new QHBoxLayout();
    auto *sliderLabel = new QLabel(QStringLiteral("Gamma:"), this);
    gammaSlider = new QSlider(Qt::Horizontal, this);
    gammaSlider->setRange(1, 50);
    gammaSlider->setValue(6);
    gammaSlider->setTickPosition(QSlider::TicksBelow);
    gammaSlider->setTickInterval(5);
    gammaValueLabel = new QLabel(QStringLiteral("0.60"), this);
    gammaValueLabel->setFixedWidth(40);

    sliderLayout->addWidget(sliderLabel);
    sliderLayout->addWidget(gammaSlider, 1);
    sliderLayout->addWidget(gammaValueLabel);

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));

    layout->addWidget(titleLabel);
    layout->addLayout(buttonLayout);
    layout->addLayout(sliderLayout);
    layout->addWidget(statusLabel);

    waitKeyTimer = new QTimer(this);
    waitKeyTimer->setInterval(30);
    connect(waitKeyTimer, &QTimer::timeout, this, []() {
        cv::waitKey(1);
    });

    connect(openButton, &QPushButton::clicked, this, &PointGrayTransformLessonWidget::openAndShow);
    connect(gammaSlider, &QSlider::valueChanged, this, &PointGrayTransformLessonWidget::updateGamma);
}

void PointGrayTransformLessonWidget::openAndShow()
{
    const QString imagePath = QStringLiteral("cat.jpg");
    originalImage = cv::imread(imagePath.toStdString(), cv::IMREAD_COLOR);
    if (originalImage.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：%1（请确认在当前目录）").arg(imagePath));
        return;
    }

    cv::cvtColor(originalImage, grayImage, cv::COLOR_BGR2GRAY);

    originalWindowName = "Original";
    processedWindowName = "Gamma Result";
    cv::namedWindow(originalWindowName, cv::WINDOW_NORMAL);
    cv::namedWindow(processedWindowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(originalWindowName, 432, 648);
    cv::resizeWindow(processedWindowName, 432, 648);
    cv::imshow(originalWindowName, originalImage);

    updateGamma(gammaSlider->value());

    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}

void PointGrayTransformLessonWidget::updateGamma(int sliderValue)
{
    if (grayImage.empty())
    {
        return;
    }

    const double gamma = static_cast<double>(sliderValue) / 10.0;
    gammaValueLabel->setText(QString::number(gamma, 'f', 2));

    const cv::Mat corrected = applyGamma(grayImage, gamma);
    cv::imshow(processedWindowName, corrected);

    QString effect;
    if (gamma < 1.0)
    {
        effect = QStringLiteral("提亮（暗部增强）");
    }
    else if (gamma > 1.0)
    {
        effect = QStringLiteral("变暗（亮部增强）");
    }
    else
    {
        effect = QStringLiteral("无变化（恒等）");
    }

    statusLabel->setText(QStringLiteral("gamma = %1 → %2").arg(gamma, 0, 'f', 2).arg(effect));
}
