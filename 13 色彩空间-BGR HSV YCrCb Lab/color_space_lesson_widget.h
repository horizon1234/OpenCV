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

class ColorSpaceLessonWidget : public QWidget
{
public:
    explicit ColorSpaceLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    std::vector<std::string> windowNames;

    void ensureTimer();
    void closeAllWindows();
    cv::Mat loadColorImage() const;

    void showBGRChannels();
    void showHSVChannels();
    void showYCrCbChannels();
    void showLabChannels();
    void showUseCaseComparison();
    void showHSVRedSegmentation();
    void showBGRTresholdPitfall();

    static cv::Mat makeGrayPreview(const cv::Mat &channel);
    static cv::Mat makeHuePreview(const cv::Mat &hChannel);
    static cv::Mat makeMaskPreview(const cv::Mat &mask);
};
