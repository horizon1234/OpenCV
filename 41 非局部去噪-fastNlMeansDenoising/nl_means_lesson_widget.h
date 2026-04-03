#pragma once

#include <QWidget>

class QLabel;
class QGridLayout;
class QTimer;

class NlMeansLessonWidget : public QWidget
{
public:
    explicit NlMeansLessonWidget(QWidget *parent = nullptr);

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
