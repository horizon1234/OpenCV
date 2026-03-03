#pragma once

#include <QWidget>

class QLabel;
class QTimer;

class ErosionBoundaryLessonWidget : public QWidget
{
public:
    explicit ErosionBoundaryLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;

    void openAndShow();
};
