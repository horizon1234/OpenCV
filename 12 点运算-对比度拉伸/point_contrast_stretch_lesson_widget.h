#pragma once

#include <QWidget>

#include <string>

class QLabel;
class QTimer;

class PointContrastStretchLessonWidget : public QWidget
{
public:
    explicit PointContrastStretchLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    std::string originalWindowName;
    std::string processedWindowName;

    void openAndShow();
};
