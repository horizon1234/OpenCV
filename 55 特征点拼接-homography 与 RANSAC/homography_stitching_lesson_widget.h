#pragma once

#include <QWidget>

class QLabel;
class QGridLayout;
class QTimer;

class HomographyStitchingLessonWidget : public QWidget
{
public:
    explicit HomographyStitchingLessonWidget(QWidget *parent = nullptr);

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
