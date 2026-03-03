#include "erosion_boundary_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

namespace
{
struct BoundaryState
{
    cv::Mat original;
    cv::Mat gray;
    cv::Mat eroded;
    cv::Mat boundary;
    std::string windowName;
    int erodeSize = 1;
};

void updateBoundary(BoundaryState *state)
{
    if (!state || state->gray.empty())
    {
        return;
    }

    const int k = state->erodeSize * 2 + 1;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(k, k));
    cv::erode(state->gray, state->eroded, kernel);
    cv::absdiff(state->gray, state->eroded, state->boundary);

    cv::imshow(state->windowName, state->boundary);
}

void onErodeTrackbar(int value, void *userdata)
{
    auto *state = static_cast<BoundaryState *>(userdata);
    if (!state)
    {
        return;
    }
    state->erodeSize = std::max(1, value);
    updateBoundary(state);
}
} // namespace

ErosionBoundaryLessonWidget::ErosionBoundaryLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("腐蚀应用：边界提取"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));

    auto *buttonLayout = new QHBoxLayout();
    auto *openButton = new QPushButton(QStringLiteral("打开并显示"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(openButton);
    buttonLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addLayout(buttonLayout);
    layout->addWidget(statusLabel);

    waitKeyTimer = new QTimer(this);
    waitKeyTimer->setInterval(30);
    connect(waitKeyTimer, &QTimer::timeout, this, []() {
        cv::waitKey(1);
    });

    connect(openButton, &QPushButton::clicked, this, &ErosionBoundaryLessonWidget::openAndShow);
}

void ErosionBoundaryLessonWidget::openAndShow()
{
    static BoundaryState state;

    const QString imagePath = QStringLiteral("cat.jpg");
    state.original = cv::imread(imagePath.toStdString(), cv::IMREAD_UNCHANGED);
    if (state.original.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：%1（请确认在当前目录）").arg(imagePath));
        return;
    }

    if (state.original.channels() == 3)
    {
        cv::cvtColor(state.original, state.gray, cv::COLOR_BGR2GRAY);
    }
    else if (state.original.channels() == 4)
    {
        cv::cvtColor(state.original, state.gray, cv::COLOR_BGRA2GRAY);
    }
    else
    {
        state.gray = state.original.clone();
    }

    state.windowName = "OpenCV Erosion Boundary";
    cv::namedWindow(state.windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(state.windowName, 432, 648);

    cv::createTrackbar("Erode", state.windowName, &state.erodeSize, 10, onErodeTrackbar, &state);

    updateBoundary(&state);

    statusLabel->setText(QStringLiteral("已显示边界：%1\n滑动 Erode 调整腐蚀核大小").arg(imagePath));
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}
