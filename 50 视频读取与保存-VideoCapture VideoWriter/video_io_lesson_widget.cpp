#include "video_io_lesson_widget.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

#include "advanced_lesson_runtime.h"
#include "advanced_lesson_widget_support.h"

namespace
{
constexpr int kLessonId = 50;
}

VideoIoLessonWidget::VideoIoLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    const auto &config = advancedLessonConfig(kLessonId);

    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QString::fromStdString(config.title), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    summaryLabel = new QLabel(QString::fromStdString(config.summary), this);
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet(QStringLiteral("color: #444; line-height: 1.5;"));

    auto *guideCard = createAdvancedLessonGuideCard(kLessonId, this);
    auto *demoNotesCard = createAdvancedLessonDemoNotesCard(kLessonId, this);

    buttonGrid = new QGridLayout();
    buttonGrid->setHorizontalSpacing(12);
    buttonGrid->setVerticalSpacing(10);
    buttonGrid->setColumnStretch(0, 1);
    buttonGrid->setColumnStretch(1, 1);

    statusLabel = new QLabel(QStringLiteral("建议先按顺序运行每个演示，并把中间窗口与状态栏提示结合起来看。"), this);
    statusLabel->setWordWrap(true);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));

    layout->addWidget(titleLabel);
    layout->addWidget(summaryLabel);
    layout->addWidget(guideCard);
    layout->addWidget(demoNotesCard);
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

void VideoIoLessonWidget::buildButtons()
{
    const auto &config = advancedLessonConfig(kLessonId);
    for (int index = 0; index < static_cast<int>(config.demos.size()); ++index)
    {
        const auto &demo = config.demos[static_cast<std::size_t>(index)];
        auto *button = new QPushButton(QString::fromStdString(demo.buttonText), this);
        button->setToolTip(QString::fromStdString(demo.description));
        button->setMinimumHeight(42);
        button->setStyleSheet(QStringLiteral("text-align: left; padding: 8px 10px;"));
        connect(button, &QPushButton::clicked, this, [this, demo]() {
            runDemo(demo.demoId);
        });

        const int row = index / 2;
        const int column = index % 2;
        buttonGrid->addWidget(button, row, column);
    }
}

void VideoIoLessonWidget::ensureTimer()
{
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}

void VideoIoLessonWidget::runDemo(int demoId)
{
    std::string statusMessage;
    closeAdvancedLessonWindows();
    if (runAdvancedLessonDemo(kLessonId, demoId, statusMessage))
    {
        ensureTimer();
    }
    statusLabel->setText(QString::fromStdString(statusMessage));
}
