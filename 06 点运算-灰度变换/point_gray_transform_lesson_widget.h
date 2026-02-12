#pragma once

#include <QWidget>

#include <opencv2/core.hpp>

#include <string>

class QLabel;
class QSlider;
class QTimer;

class PointGrayTransformLessonWidget : public QWidget
{
public:
    explicit PointGrayTransformLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QLabel *gammaValueLabel = nullptr;
    QSlider *gammaSlider = nullptr;
    QTimer *waitKeyTimer = nullptr;
    cv::Mat originalImage;
    cv::Mat grayImage;
    std::string originalWindowName;
    std::string processedWindowName;

    void openAndShow();
    void updateGamma(int sliderValue);
};
