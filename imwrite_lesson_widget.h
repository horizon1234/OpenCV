#pragma once

#include <QWidget>
#include <QLabel>
#include <opencv2/opencv.hpp>

class ImwriteLessonWidget : public QWidget
{
public:
    explicit ImwriteLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *imageLabel = nullptr;
    QLabel *statusLabel = nullptr;

    static cv::Mat generateImage();
    void generateAndShowImage();
};
