#pragma once

#include <QWidget>

#include <string>

class QLabel;
class QTimer;

class PointInvertLessonWidget : public QWidget
{
public:
    explicit PointInvertLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;
    std::string originalWindowName;
    std::string processedWindowName;

    void openAndShow();
};
