#pragma once

#include <QLabel>
#include <QWidget>
#include <QString>
#include <vector>
#include <opencv2/opencv.hpp>

class QTimer;

class ImwriteLessonWidget : public QWidget
{
public:
    explicit ImwriteLessonWidget(QWidget *parent = nullptr);
    ~ImwriteLessonWidget() override;

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    cv::Mat sourceImage;
    QString sourceImagePath;
    std::vector<std::string> windowNames;

    static QString formatFileSize(qint64 bytes);
    void loadSourceImage();
    void ensureTimer();
    void closeAllWindows();
    void showPngComparison();
    void showJpegComparison();
    void showWebpComparison();
    void showAllComparisons();
};
