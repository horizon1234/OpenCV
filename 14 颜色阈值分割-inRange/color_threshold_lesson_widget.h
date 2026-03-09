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

class ColorThresholdLessonWidget : public QWidget
{
public:
    explicit ColorThresholdLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    std::vector<std::string> windowNames;

    void ensureTimer();
    void closeAllWindows();
    cv::Mat loadColorImage() const;

    void showSingleHSVRange();
    void showRedDualRange();
    void showMaskCleanup();
    void showBGRvsHSV();
    void showHSVTrackbarTuning();

    static cv::Mat maskToBgr(const cv::Mat &mask);
};
