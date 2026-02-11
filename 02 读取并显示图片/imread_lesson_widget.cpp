#include "imread_lesson_widget.h"

#include <QByteArray>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include <cstring>

#include <opencv2/opencv.hpp>

#include "../mat_to_qimage.h"

ImreadLessonWidget::ImreadLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("imread 学习：读取并显示图片"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(640, 480);

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));

    auto *buttonLayout = new QHBoxLayout();
    auto *reloadButton = new QPushButton(QStringLiteral("重新读取"), this);
    auto *showNormalButton = new QPushButton(QStringLiteral("显示正常"), this);
    auto *showWrongStepButton = new QPushButton(QStringLiteral("显示错位示例"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(reloadButton);
    buttonLayout->addWidget(showNormalButton);
    buttonLayout->addWidget(showWrongStepButton);
    buttonLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addWidget(imageLabel, 1);
    layout->addLayout(buttonLayout);
    layout->addWidget(statusLabel);

    connect(reloadButton, &QPushButton::clicked, this, [this]() {
        loadAndShowImage();
    });
    connect(showNormalButton, &QPushButton::clicked, this, [this]() {
        if (!correctImage.isNull())
        {
            imageLabel->setPixmap(QPixmap::fromImage(correctImage));
            statusLabel->setText(statusText + QStringLiteral("\n当前显示：正常 step"));
        }
    });
    connect(showWrongStepButton, &QPushButton::clicked, this, [this]() {
        if (!wrongStepImage.isNull())
        {
            imageLabel->setPixmap(QPixmap::fromImage(wrongStepImage));
            statusLabel->setText(statusText + QStringLiteral("\n当前显示：错误 step（错位示例）"));
        }
    });

    loadAndShowImage();
}

void ImreadLessonWidget::loadAndShowImage()
{
    const QString imagePath = QStringLiteral("cat.jpg");
    const QString cwd = QDir::currentPath();
    const QString absPath = QFileInfo(imagePath).absoluteFilePath();
    const bool fileExists = QFileInfo::exists(absPath);

    // 原注释（保留）：
    // cv::Mat就是OpenCV中的图像数据结构，读图、处理、存图都离不开它。
    // 常用的枚举参数有：
    // cv::IMREAD_UNCHANGED - 包含Alpha通道，按原始格式读取
    // cv::IMREAD_GRAYSCALE - 灰度图
    // cv::IMREAD_COLOR - 彩色图，忽略Alpha通道

    // 这里读取为灰度图，方便演示 step 设置错误导致的错位
    const cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_GRAYSCALE);
    if (image.empty())
    {
        statusText = QStringLiteral("读取失败：%1\n工作目录：%2\n解析路径：%3\n文件存在：%4")
                         .arg(imagePath)
                         .arg(cwd)
                         .arg(absPath)
                         .arg(fileExists ? QStringLiteral("是") : QStringLiteral("否"));
        statusLabel->setText(statusText);
        imageLabel->clear();
        return;
    }

    const QImage qimage = matToQImage(image);
    if (qimage.isNull())
    {
        QMessageBox::critical(this,
                              QStringLiteral("Image Format Error"),
                              QStringLiteral("Unsupported image format from OpenCV."));
        return;
    }

    correctImage = qimage;

    const int wrongBytesPerLine = image.cols + 16;
    const int totalBytes = wrongBytesPerLine * image.rows;
    QByteArray wrongBuffer(totalBytes, 0);
    for (int y = 0; y < image.rows; ++y)
    {
        const uchar *src = image.ptr<uchar>(y);
        uchar *dst = reinterpret_cast<uchar *>(wrongBuffer.data() + y * image.cols);
        std::memcpy(dst, src, static_cast<size_t>(image.cols));
    }
    QImage wrong(reinterpret_cast<const uchar *>(wrongBuffer.constData()),
                 image.cols,
                 image.rows,
                 wrongBytesPerLine,
                 QImage::Format_Grayscale8);
    wrongStepImage = wrong.copy();

    statusText = QStringLiteral("读取成功：%1\n工作目录：%2\n解析路径：%3\n正确 step：%4 字节/行\n错误 step：%5 字节/行")
                     .arg(imagePath)
                     .arg(cwd)
                     .arg(absPath)
                     .arg(static_cast<int>(image.step))
                     .arg(wrongBytesPerLine);

    statusLabel->setText(statusText + QStringLiteral("\n当前显示：正常 step"));
    imageLabel->setPixmap(QPixmap::fromImage(correctImage));
}
