#pragma once

#include <QWidget>

#include <opencv2/core.hpp>

#include <string>
#include <vector>

class QLabel;
class QTimer;

class PointHistogramLessonWidget : public QWidget
{
public:
    explicit PointHistogramLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    std::vector<std::string> windowNames;

    void ensureTimer();
    void closeAllWindows();

    // 原有演示：YCrCb 空间均衡化（正确做法）
    void showYCrCbEqualization();
    // 错误示例：直接对 BGR 三通道分别均衡化
    void showWrongBGREqualization();
    // CLAHE 自适应直方图均衡化
    void showCLAHE();
    // 实验1：可视化直方图（均衡化前后）
    void showHistogramVisualization();
    // 实验2：全局均衡化 vs CLAHE 对比
    void showGlobalVsCLAHE();

    // 辅助：绘制灰度直方图到 cv::Mat
    static cv::Mat drawHistogram(const cv::Mat &gray, const cv::Scalar &color);
};
