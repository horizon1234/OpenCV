#include "morphology_trackbar_lesson_widget.h"

#include <algorithm>
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
    int kernelShape = cv::MORPH_RECT; // 结构元素形状
    int operation = 0; // 0: erode+dilate 1: open 2: close 3: gradient
};

MorphologyState *gState = nullptr;
MorphologyTrackbarLessonWidget *gWidget = nullptr;

QString modeName(int mode)
{
    switch (mode)
    {
    case 0:
        return QStringLiteral("彩色");
    case 1:
        return QStringLiteral("灰度");
    case 2:
        return QStringLiteral("二值");
    default:
        return QStringLiteral("未知");
    }
}

QString kernelShapeName(int kernelShape)
{
    switch (kernelShape)
    {
    case cv::MORPH_RECT:
        return QStringLiteral("矩形核");
    case cv::MORPH_CROSS:
        return QStringLiteral("十字核");
    case cv::MORPH_ELLIPSE:
        return QStringLiteral("椭圆核");
    default:
        return QStringLiteral("未知核");
    }
}

QString operationName(int operation)
{
    switch (operation)
    {
    case 0:
        return QStringLiteral("基础模式：先腐蚀再膨胀");
    case 1:
        return QStringLiteral("开运算：先腐蚀后膨胀");
    case 2:
        return QStringLiteral("闭运算：先膨胀后腐蚀");
    case 3:
        return QStringLiteral("形态学梯度：膨胀 - 腐蚀");
    default:
        return QStringLiteral("未知操作");
    }
}

cv::Mat buildKernel(const MorphologyState *state, int radius)
{
    const int k = radius * 2 + 1;
    return cv::getStructuringElement(state->kernelShape, cv::Size(k, k));
}

QString buildStatusText(const MorphologyState *state)
{
    if (!state || state->original.empty())
    {
        return QStringLiteral("尚未加载图片。");
    }

    const int erodeKernel = state->erodeSize * 2 + 1;
    const int dilateKernel = state->dilateSize * 2 + 1;
    return QStringLiteral("当前模式：%1\n当前核形状：%2\n当前操作：%3\n腐蚀半径：%4（核大小 %5×%5）\n膨胀半径：%6（核大小 %7×%7）\n说明：腐蚀取局部最小值，膨胀取局部最大值；开闭运算和梯度可用来验证形态学的组合性质。"
                             )
        .arg(modeName(state->mode))
        .arg(kernelShapeName(state->kernelShape))
        .arg(operationName(state->operation))
        .arg(state->erodeSize)
        .arg(erodeKernel)
        .arg(state->dilateSize)
        .arg(dilateKernel);
}

void refreshWidgetStatus()
{
    if (gWidget)
    {
        gWidget->updateStatusText();
    }
}

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

    if (state->operation == 0)
    {   
        if (state->erodeSize > 0)
        {
            cv::erode(state->display, state->display, buildKernel(state, state->erodeSize));
        }

        if (state->dilateSize > 0)
        {
            cv::dilate(state->display, state->display, buildKernel(state, state->dilateSize));
        }
    }
    else if (state->operation == 1)
    {
        const int radius = std::max(state->erodeSize, state->dilateSize);
        if (radius > 0)
        {
            cv::morphologyEx(state->display, state->display, cv::MORPH_OPEN, buildKernel(state, radius));
        }
    }
    else if (state->operation == 2)
    {
        const int radius = std::max(state->erodeSize, state->dilateSize);
        if (radius > 0)
        {
            cv::morphologyEx(state->display, state->display, cv::MORPH_CLOSE, buildKernel(state, radius));
        }
    }
    else if (state->operation == 3)
    {
        const int radius = std::max(state->erodeSize, state->dilateSize);
        if (radius > 0)
        {
            cv::morphologyEx(state->display, state->display, cv::MORPH_GRADIENT, buildKernel(state, radius));
        }
    }

    cv::imshow(state->windowName, state->display);   // 显示处理后的图像
    refreshWidgetStatus();
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

void setKernelShape(MorphologyState *state, int kernelShape)
{
    if (!state)
    {
        return;
    }
    state->kernelShape = kernelShape;
    applyMorphology(state);
}

void setOperation(MorphologyState *state, int operation)
{
    if (!state)
    {
        return;
    }
    state->operation = operation;
    applyMorphology(state);
}
} // namespace

MorphologyTrackbarLessonWidget::MorphologyTrackbarLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    gWidget = this;
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

    auto *kernelLayout = new QHBoxLayout();
    auto *rectButton = new QPushButton(QStringLiteral("矩形核"), this);
    auto *crossButton = new QPushButton(QStringLiteral("十字核"), this);
    auto *ellipseButton = new QPushButton(QStringLiteral("椭圆核"), this);
    kernelLayout->addStretch();
    kernelLayout->addWidget(rectButton);
    kernelLayout->addWidget(crossButton);
    kernelLayout->addWidget(ellipseButton);
    kernelLayout->addStretch();

    auto *operationLayout = new QHBoxLayout();
    auto *baseButton = new QPushButton(QStringLiteral("基础腐蚀/膨胀"), this);
    auto *openOpButton = new QPushButton(QStringLiteral("开运算"), this);
    auto *closeOpButton = new QPushButton(QStringLiteral("闭运算"), this);
    auto *gradientButton = new QPushButton(QStringLiteral("形态学梯度"), this);
    operationLayout->addStretch();
    operationLayout->addWidget(baseButton);
    operationLayout->addWidget(openOpButton);
    operationLayout->addWidget(closeOpButton);
    operationLayout->addWidget(gradientButton);
    operationLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addLayout(buttonLayout);
    layout->addLayout(kernelLayout);
    layout->addLayout(operationLayout);
    layout->addWidget(statusLabel);
    statusLabel->setWordWrap(true);

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
    connect(rectButton, &QPushButton::clicked, this, []() {
        setKernelShape(gState, cv::MORPH_RECT);
    });
    connect(crossButton, &QPushButton::clicked, this, []() {
        setKernelShape(gState, cv::MORPH_CROSS);
    });
    connect(ellipseButton, &QPushButton::clicked, this, []() {
        setKernelShape(gState, cv::MORPH_ELLIPSE);
    });
    connect(baseButton, &QPushButton::clicked, this, []() {
        setOperation(gState, 0);
    });
    connect(openOpButton, &QPushButton::clicked, this, []() {
        setOperation(gState, 1);
    });
    connect(closeOpButton, &QPushButton::clicked, this, []() {
        setOperation(gState, 2);
    });
    connect(gradientButton, &QPushButton::clicked, this, []() {
        setOperation(gState, 3);
    });
}

MorphologyTrackbarLessonWidget::~MorphologyTrackbarLessonWidget()
{
    if (gWidget == this)
    {
        gWidget = nullptr;
    }
    if (gState)
    {
        cv::destroyWindow(gState->windowName);
    }
}

void MorphologyTrackbarLessonWidget::updateStatusText() const
{
    statusLabel->setText(buildStatusText(gState));
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

    updateStatusText();
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}
