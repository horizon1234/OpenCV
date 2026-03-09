#pragma once

#include <QWidget>

#include <string>
#include <vector>

class QLabel;
class QDialog;
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
    QDialog *diagramDialog = nullptr;
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
    void showHSVConeDiagram();
    void showHueWheelDiagram();
    void showColorSpaceMapDiagram();
    void showBGRRBoostDiagram();
    void showDiagramPreview(const QString &fileName, const QString &title, const QString &description);

    static cv::Mat makeGrayPreview(const cv::Mat &channel);
    static cv::Mat makeHuePreview(const cv::Mat &hChannel);
    static cv::Mat makeMaskPreview(const cv::Mat &mask);
};
