#pragma once

#include <QWidget>

#include <opencv2/core.hpp>

#include <string>

class QLabel;
class QTimer;

class PointTruncationLessonWidget : public QWidget
{
public:
    explicit PointTruncationLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    std::string originalWindowName;
    std::string processedWindowName;

    void openAndShow();
};
