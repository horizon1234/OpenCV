#include "morphology_trackbar_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

namespace
{
struct MorphologyState
{
    cv::Mat original;   // 原始图像
    cv::Mat display;    // 显示的图像
    std::string windowName; // 窗口名称
    int erodeSize = 0;  // 腐蚀大小
    int dilateSize = 0; // 膨胀大小
    int mode = 0; // 0: 彩色 1: 灰度 2: 二值
};

MorphologyState *gState = nullptr;

void applyMorphology(MorphologyState *state)
{
    if (!state || state->original.empty())
    {
        return;
    }

    // 根据模式转换图像：0 彩色、1 灰度、2 二值
    if (state->mode == 1)
    {
        cv::cvtColor(state->original, state->display, cv::COLOR_BGR2GRAY);
    }
    else if (state->mode == 2)
    {
        cv::Mat gray;
        cv::cvtColor(state->original, gray, cv::COLOR_BGR2GRAY);
        cv::threshold(gray, state->display, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    }
    else
    {
        state->display = state->original.clone();   // 重置为原始图像
    }

    if (state->erodeSize > 0)   // 应用腐蚀，它的效果是让亮区域(前景)变小，暗区域变大，能够去除小的白色噪点，分开连接在一起的物体。
    {   
        // 腐蚀核越大，图片越暗淡
        const int k = state->erodeSize * 2 + 1; // 计算核大小
        // 创建矩形结构元素作为腐蚀核，其中函数getStructuringElement的名字含义是“获取结构元素”，第一个参数指定形状，第二个参数指定大小。
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(k, k));
        // 执行腐蚀操作，参数依次为：输入图像、输出图像、腐蚀核
        // 把每个像素替换成其领域内的最小值，所以亮区域会变小，暗区域会变大。核越大，被替换的范围越大，效果越明显。
        cv::erode(state->display, state->display, kernel);
    }

    if (state->dilateSize > 0)   // 应用膨胀，它的效果是让亮区域(前景)变大，暗区域变小，能够填补小的黑色孔洞，连接断开的物体。
    {
        // 膨胀核越大，图片越明亮
        const int k = state->dilateSize * 2 + 1; // 计算核大小
        // 创建矩形结构元素作为膨胀核，其中函数getStructuringElement的名字含义是“获取结构元素”，第一个参数指定形状，第二个参数指定大小。
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(k, k));
        // 执行膨胀操作，参数依次为：输入图像、输出图像、膨胀核
        // 把每个像素替换成其领域内的最大值，所以亮区域会变大，暗区域会变小。核越大，被替换的范围越大，效果越明显。
        cv::dilate(state->display, state->display, kernel);
    }

    cv::imshow(state->windowName, state->display);   // 显示处理后的图像
}

// 回调函数：处理腐蚀滑动条变化
void onErodeTrackbar(int value, void *userdata)
{
    auto *state = static_cast<MorphologyState *>(userdata);
    if (!state)
    {
        return;
    }
    state->erodeSize = value;
    applyMorphology(state);
}

// 回调函数：处理膨胀滑动条变化
void onDilateTrackbar(int value, void *userdata)
{
    auto *state = static_cast<MorphologyState *>(userdata);
    if (!state)
    {
        return;
    }
    state->dilateSize = value;
    applyMorphology(state);
}
} // namespace

MorphologyTrackbarLessonWidget::MorphologyTrackbarLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("滑动条：腐蚀与膨胀"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));

    auto *buttonLayout = new QHBoxLayout();
    auto *openButton = new QPushButton(QStringLiteral("打开并显示"), this);
    auto *colorButton = new QPushButton(QStringLiteral("彩色图"), this);
    auto *grayButton = new QPushButton(QStringLiteral("灰度图"), this);
    auto *binaryButton = new QPushButton(QStringLiteral("二值图"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(openButton);
    buttonLayout->addWidget(colorButton);
    buttonLayout->addWidget(grayButton);
    buttonLayout->addWidget(binaryButton);
    buttonLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addLayout(buttonLayout);
    layout->addWidget(statusLabel);

    waitKeyTimer = new QTimer(this);
    waitKeyTimer->setInterval(30);
    connect(waitKeyTimer, &QTimer::timeout, this, []() {
        cv::waitKey(1);
    });

    connect(openButton, &QPushButton::clicked, this, &MorphologyTrackbarLessonWidget::openAndShow);
    connect(colorButton, &QPushButton::clicked, this, []() {
        if (gState)
        {
            gState->mode = 0;
            applyMorphology(gState);
        }
    });
    connect(grayButton, &QPushButton::clicked, this, []() {
        if (gState)
        {
            gState->mode = 1;
            applyMorphology(gState);
        }
    });
    connect(binaryButton, &QPushButton::clicked, this, []() {
        if (gState)
        {
            gState->mode = 2;
            applyMorphology(gState);
        }
    });
}

void MorphologyTrackbarLessonWidget::openAndShow()
{
    // 使用静态变量以保持状态，避免每次调用都重新创建
    static MorphologyState state;
    gState = &state;

    const QString imagePath = QStringLiteral("cat.jpg");
    state.original = cv::imread(imagePath.toStdString(), cv::IMREAD_UNCHANGED);
    if (state.original.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：%1（请确认在当前目录）").arg(imagePath));
        return;
    }

    state.windowName = "OpenCV Morphology Trackbar";
    cv::namedWindow(state.windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(state.windowName, 432, 648);
    cv::imshow(state.windowName, state.original);

    // 创建腐蚀和膨胀的滑动条，并关联回调函数
    // 参数依次为：滑动条名称、窗口名称、变量地址、最大值、回调函数、用户数据
    cv::createTrackbar("Erode", state.windowName, &state.erodeSize, 10, onErodeTrackbar, &state);
    cv::createTrackbar("Dilate", state.windowName, &state.dilateSize, 10, onDilateTrackbar, &state);

    // 初始应用一次形态学操作以显示效果
    applyMorphology(&state);

    statusLabel->setText(QStringLiteral("已显示：%1\n拖动滑动条控制腐蚀/膨胀").arg(imagePath));
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}
