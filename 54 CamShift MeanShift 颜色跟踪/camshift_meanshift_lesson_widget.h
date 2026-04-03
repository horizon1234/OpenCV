#pragma once

#include <QWidget>

class QLabel;
class QGridLayout;
class QTimer;

class CamshiftMeanshiftLessonWidget : public QWidget
{
public:
    explicit CamshiftMeanshiftLessonWidget(QWidget *parent = nullptr);

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
