#pragma once

#include <QWidget>

class QLabel;
class QGridLayout;
class QTimer;

class CameraCalibrationLessonWidget : public QWidget
{
public:
    explicit CameraCalibrationLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *summaryLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QGridLayout *buttonGrid = nullptr;
    QTimer *waitKeyTimer = nullptr;

    void buildButtons();
    void ensureTimer();
    void runDemo(int demoId);
};
