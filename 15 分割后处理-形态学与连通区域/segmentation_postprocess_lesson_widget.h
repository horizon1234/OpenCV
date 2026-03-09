#pragma once

#include <QWidget>

#include <string>
#include <vector>

class QLabel;
class QTimer;

namespace cv
{
class Mat;
}

class SegmentationPostprocessLessonWidget : public QWidget
{
public:
    explicit SegmentationPostprocessLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    std::vector<std::string> windowNames;

    void ensureTimer();
    void closeAllWindows();
    cv::Mat loadColorImage() const;
    cv::Mat buildRawMask(const cv::Mat &color) const;

    void showRawVsOpenClose();
    void showKernelTrackbarTuning();
    void showMorphologyGradient();
    void showLargestComponent();
    void showFullPipeline();

    static cv::Mat maskToBgr(const cv::Mat &mask);
};
