#pragma once

#include <QWidget>

#include <string>

class QLabel;
class QTimer;

class PointColorAdjustLessonWidget : public QWidget
{
public:
    explicit PointColorAdjustLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    std::string originalWindowName;
    std::string processedWindowName;

    void openAndShow();
};
