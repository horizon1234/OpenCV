#include "imwrite_lesson_widget.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include "../mat_to_qimage.h"

ImwriteLessonWidget::ImwriteLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("imwrite 学习：生成并保存图片"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(640, 480);

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));

    auto *buttonLayout = new QHBoxLayout();
    auto *regenerateButton = new QPushButton(QStringLiteral("重新生成并保存"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(regenerateButton);
    buttonLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addWidget(imageLabel, 1);
    layout->addLayout(buttonLayout);
    layout->addWidget(statusLabel);

    connect(regenerateButton, &QPushButton::clicked, this, [this]() {
        generateAndShowImage();
    });

    generateAndShowImage();
}

cv::Mat ImwriteLessonWidget::generateImage()
{
    cv::Mat image(480, 640, CV_8UC4);
    for (int y = 0; y < image.rows; ++y)
    {
        for (int x = 0; x < image.cols; ++x)
        {
            cv::Vec4b &bgra = image.at<cv::Vec4b>(y, x);
            bgra[0] = static_cast<uchar>(255 * x / image.cols);
            bgra[1] = static_cast<uchar>(255 * y / image.rows);
            bgra[2] = static_cast<uchar>(255 - bgra[0]);
            bgra[3] = static_cast<uchar>(200);
        }
    }
    return image;
}

void ImwriteLessonWidget::generateAndShowImage()
{
    const cv::Mat image = generateImage();
    const std::string outputPath = "generated_from_imwrite.png";

    if (!cv::imwrite(outputPath, image))
    {
        statusLabel->setText(QStringLiteral("保存失败：%1").arg(QString::fromStdString(outputPath)));
    }
    else
    {
        statusLabel->setText(QStringLiteral("已保存到项目根目录：%1").arg(QString::fromStdString(outputPath)));
    }

    const QImage qimage = matToQImage(image);
    if (qimage.isNull())
    {
        QMessageBox::critical(this,
                              QStringLiteral("Image Format Error"),
                              QStringLiteral("Unsupported image format from OpenCV."));
        return;
    }

    imageLabel->setPixmap(QPixmap::fromImage(qimage));
}
