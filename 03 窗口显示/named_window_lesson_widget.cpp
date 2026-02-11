#include "named_window_lesson_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

// 提取 OpenCV 构建信息中的 GUI 后端信息
static QString extractGuiBackend()
{
    // 获取完整的构建信息
    const std::string info = cv::getBuildInformation();
    const QStringList lines = QString::fromStdString(info).split('\n');
    for (const QString &line : lines)
    {
        if (line.trimmed().startsWith(QStringLiteral("GUI:")))
        {
            return line.trimmed();
        }
    }
    return QStringLiteral("GUI: unknown");
}

// 将鼠标事件代码转换为可读字符串
static QString mouseEventName(int event)
{
    switch (event)
    {
    case cv::EVENT_MOUSEMOVE:
        return QStringLiteral("移动");
    case cv::EVENT_LBUTTONDOWN:
        return QStringLiteral("左键按下");
    case cv::EVENT_LBUTTONUP:
        return QStringLiteral("左键抬起");
    case cv::EVENT_RBUTTONDOWN:
        return QStringLiteral("右键按下");
    case cv::EVENT_RBUTTONUP:
        return QStringLiteral("右键抬起");
    case cv::EVENT_MBUTTONDOWN:
        return QStringLiteral("中键按下");
    case cv::EVENT_MBUTTONUP:
        return QStringLiteral("中键抬起");
    case cv::EVENT_LBUTTONDBLCLK:
        return QStringLiteral("左键双击");
    case cv::EVENT_RBUTTONDBLCLK:
        return QStringLiteral("右键双击");
    case cv::EVENT_MBUTTONDBLCLK:
        return QStringLiteral("中键双击");
    default:
        return QStringLiteral("未知事件");
    }
}

// 鼠标回调函数，其中的参数含义分别为：事件类型、x坐标、y坐标、事件标志、用户数据指针
static void onMouseCallback(int event, int x, int y, int flags, void *userdata)
{
    auto *self = static_cast<NamedWindowLessonWidget *>(userdata);
    if (!self)
    {
        return;
    }

    const QString text = QStringLiteral("鼠标事件：%1  x=%2  y=%3  flags=%4")
                             .arg(mouseEventName(event))
                             .arg(x)
                             .arg(y)
                             .arg(flags);
    
    // 使用 Qt 的线程安全机制将更新操作放到主线程执行                         
    QMetaObject::invokeMethod(self, [self, text]() {
        self->updateMouseStatus(text);
    }, Qt::QueuedConnection);

    QMetaObject::invokeMethod(self, [self, event, x, y, flags]() {
        self->handleMouseEvent(event, x, y, flags);
    }, Qt::QueuedConnection);
}

NamedWindowLessonWidget::NamedWindowLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("namedWindow 学习：OpenCV 窗口显示"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));

    auto *buttonLayout = new QHBoxLayout();
    auto *openButton = new QPushButton(QStringLiteral("打开并显示"), this);
    auto *clearButton = new QPushButton(QStringLiteral("清空画布"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(openButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();

    auto *brushLayout = new QHBoxLayout();
    auto *redButton = new QPushButton(QStringLiteral("红色"), this);
    auto *greenButton = new QPushButton(QStringLiteral("绿色"), this);
    auto *blueButton = new QPushButton(QStringLiteral("蓝色"), this);
    auto *thicknessLabel = new QLabel(QStringLiteral("粗细"), this);
    thicknessSlider = new QSlider(Qt::Horizontal, this);
    thicknessSlider->setRange(1, 10);
    thicknessSlider->setValue(2);
    thicknessSlider->setFixedWidth(120);

    brushLayout->addStretch();
    brushLayout->addWidget(redButton);
    brushLayout->addWidget(greenButton);
    brushLayout->addWidget(blueButton);
    brushLayout->addSpacing(12);
    brushLayout->addWidget(thicknessLabel);
    brushLayout->addWidget(thicknessSlider);
    brushLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addLayout(buttonLayout);
    layout->addLayout(brushLayout);
    layout->addWidget(statusLabel);

    waitKeyTimer = new QTimer(this);
    waitKeyTimer->setInterval(30);
    connect(waitKeyTimer, &QTimer::timeout, this, []() {
        cv::waitKey(1);
    });

    connect(openButton, &QPushButton::clicked, this, &NamedWindowLessonWidget::openAndShow);
    connect(clearButton, &QPushButton::clicked, this, &NamedWindowLessonWidget::resetCanvas);
    connect(redButton, &QPushButton::clicked, this, [this]() {
        brushColor = cv::Scalar(0, 0, 255, 255);
    });
    connect(greenButton, &QPushButton::clicked, this, [this]() {
        brushColor = cv::Scalar(0, 255, 0, 255);
    });
    connect(blueButton, &QPushButton::clicked, this, [this]() {
        brushColor = cv::Scalar(255, 0, 0, 255);
    });
    connect(thicknessSlider, &QSlider::valueChanged, this, [this](int value) {
        brushThickness = value;
    });
}

void NamedWindowLessonWidget::openAndShow()
{
    const QString imagePath = QStringLiteral("cat.jpg");
    // 使用 IMREAD_UNCHANGED 以保留图像的原始通道和深度
    originalImage = cv::imread(imagePath.toStdString(), cv::IMREAD_UNCHANGED);
    if (originalImage.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：%1（请确认在当前目录）").arg(imagePath));
        return;
    }

    displayImage = originalImage.clone();

    windowName = "OpenCV namedWindow";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 432, 648);
    // cv::imshow 会自动创建窗口，但这里为了演示 namedWindow，先调用它
    // cv::namedWindow是OpenCV中用于创建一个窗口以显示图像的函数，它底层使用系统原生的GUI能力，比如X11/GTK等后端之一。
    cv::imshow(windowName, displayImage);

    // 设置鼠标回调以捕获鼠标事件
    cv::setMouseCallback(windowName, onMouseCallback, this);

    // 提取并显示 GUI 后端信息
    const QString guiBackend = extractGuiBackend();
    baseStatusText = QStringLiteral("已在 OpenCV 窗口显示：%1\n%2").arg(imagePath, guiBackend);
    statusLabel->setText(baseStatusText);
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}

void NamedWindowLessonWidget::updateMouseStatus(const QString &mouseText)
{
    if (baseStatusText.isEmpty())
    {
        statusLabel->setText(mouseText);
        return;
    }

    statusLabel->setText(baseStatusText + QStringLiteral("\n") + mouseText);
}

// 清空
void NamedWindowLessonWidget::resetCanvas()
{
    if (windowName.empty() || originalImage.empty())
    {
        return;
    }

    displayImage = originalImage.clone();
    cv::imshow(windowName, displayImage);
}

// 处理鼠标事件以实现绘图功能
void NamedWindowLessonWidget::handleMouseEvent(int event, int x, int y, int flags)
{
    if (windowName.empty() || displayImage.empty())
    {
        return;
    }

    // 当前鼠标位置
    const cv::Point currentPoint(x, y);

    // 当左键按下时开始绘图
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        isDrawing = true;
        lastPoint = currentPoint;
        return;
    }

    // 当鼠标移动时，如果正在绘图，则绘制线条
    if (event == cv::EVENT_MOUSEMOVE)
    {
        if (isDrawing)
        {
            // 使用抗锯齿线条进行绘制
            // 各个参数依次为：图像、起点、终点、颜色、粗细、线型，cv::line会直接修改displayImage的Mat数据，无法恢复。
            cv::line(displayImage, lastPoint, currentPoint, brushColor, brushThickness, cv::LINE_AA);
            lastPoint = currentPoint;
            cv::imshow(windowName, displayImage);
        }
        return;
    }

    // 当左键释放时停止绘图
    if (event == cv::EVENT_LBUTTONUP)
    {
        isDrawing = false;
    }
}
