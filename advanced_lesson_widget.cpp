#include "advanced_lesson_widget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

#include "advanced_lesson_runtime.h"

AdvancedLessonWidget::AdvancedLessonWidget(int lessonId, QWidget *parent)
    : QWidget(parent)
    , lessonId(lessonId)
{
    const auto &config = advancedLessonConfig(lessonId);

    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QString::fromStdString(config.title), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    summaryLabel = new QLabel(QString::fromStdString(config.summary), this);
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet(QStringLiteral("color: #444; line-height: 1.5;"));

    buttonGrid = new QGridLayout();
    buttonGrid->setHorizontalSpacing(12);
    buttonGrid->setVerticalSpacing(10);

    statusLabel = new QLabel(QStringLiteral("建议先按顺序看核心演示，再看参数对比和进阶演示。"), this);
    statusLabel->setWordWrap(true);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));

    layout->addWidget(titleLabel);
    layout->addWidget(summaryLabel);
    layout->addLayout(buttonGrid);
    layout->addWidget(statusLabel);
    layout->addStretch();

    waitKeyTimer = new QTimer(this);
    waitKeyTimer->setInterval(30);
    connect(waitKeyTimer, &QTimer::timeout, this, []() {
        cv::waitKey(1);
    });

    buildButtons();
}

void AdvancedLessonWidget::buildButtons()
{
    const auto &config = advancedLessonConfig(lessonId);
    for (int index = 0; index < static_cast<int>(config.demos.size()); ++index)
    {
        const auto &demo = config.demos[index];
        auto *button = new QPushButton(QString::fromStdString(demo.buttonText), this);
        button->setToolTip(QString::fromStdString(demo.description));
        connect(button, &QPushButton::clicked, this, [this, demo]() {
            runDemo(demo.demoId);
        });

        const int row = index / 2;
        const int column = index % 2;
        buttonGrid->addWidget(button, row, column);
    }
}

void AdvancedLessonWidget::ensureTimer()
{
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}

void AdvancedLessonWidget::runDemo(int demoId)
{
    std::string statusMessage;
    closeAdvancedLessonWindows();
    if (runAdvancedLessonDemo(lessonId, demoId, statusMessage))
    {
        ensureTimer();
    }
    statusLabel->setText(QString::fromStdString(statusMessage));
}