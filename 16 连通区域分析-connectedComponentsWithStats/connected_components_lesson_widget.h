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

class ConnectedComponentsLessonWidget : public QWidget
{
public:
    explicit ConnectedComponentsLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    std::vector<std::string> windowNames;

    void ensureTimer();
    void closeAllWindows();
    cv::Mat loadColorImage() const;
    cv::Mat buildRawMask(const cv::Mat &color) const;

    void showComponentLabels();
    void showAreaFiltering();
    void showLargestComponent();
    void showStatsOverlay();

    static cv::Mat maskToBgr(const cv::Mat &mask);
    static cv::Mat colorizeLabels(const cv::Mat &labels, int numLabels);
};
