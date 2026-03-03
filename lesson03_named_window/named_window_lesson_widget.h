#pragma once

#include <QWidget>

#include <QString>
#include <string>

#include <opencv2/opencv.hpp>

class QLabel;
class QTimer;
class QSlider;

class NamedWindowLessonWidget : public QWidget
{
public:
    explicit NamedWindowLessonWidget(QWidget *parent = nullptr);
    void updateMouseStatus(const QString &mouseText);
    void handleMouseEvent(int event, int x, int y, int flags);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    QSlider *thicknessSlider = nullptr;
    std::string windowName;
    QString baseStatusText;
    cv::Mat originalImage;
    cv::Mat displayImage;
    bool isDrawing = false;
    cv::Point lastPoint;
    cv::Scalar brushColor = cv::Scalar(0, 0, 255, 255);
    int brushThickness = 2;

    void openAndShow();
    void resetCanvas();
};
