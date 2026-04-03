#pragma once

#include <QWidget>

class QLabel;
class QGridLayout;
class QTimer;

class AdvancedLessonWidget : public QWidget
{
public:
    explicit AdvancedLessonWidget(int lessonId, QWidget *parent = nullptr);

private:
    int lessonId;
    QLabel *titleLabel = nullptr;
    QLabel *summaryLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QGridLayout *buttonGrid = nullptr;
    QTimer *waitKeyTimer = nullptr;

    void buildButtons();
    void ensureTimer();
    void runDemo(int demoId);
};