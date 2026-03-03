#pragma once

#include <QWidget>

#include <string>

class QLabel;
class QTimer;

class PointThresholdLessonWidget : public QWidget
{
public:
    explicit PointThresholdLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    std::string originalWindowName;
    std::string processedWindowName;

    void openAndShow();
};
