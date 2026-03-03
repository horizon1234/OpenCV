#pragma once

#include <QWidget>

class QLabel;
class QTimer;

class MorphologyTrackbarLessonWidget : public QWidget
{
public:
    explicit MorphologyTrackbarLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QTimer *waitKeyTimer = nullptr;

    void openAndShow();
};
