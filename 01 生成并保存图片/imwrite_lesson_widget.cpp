#include "imwrite_lesson_widget.h"

#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>

namespace
{
struct VariantJob
{
    QString fileName;
    QString label;
    std::vector<int> params;
};

struct VariantResult
{
    QString fileName;
    QString label;
    cv::Mat image;
    qint64 fileSize = -1;
};

cv::Mat saveAndReloadVariant(const cv::Mat &source,
                            const VariantJob &job,
                            VariantResult *result)
{
    if (result)
    {
        result->fileName = job.fileName;
        result->label = job.label;
        result->fileSize = -1;
        result->image.release();
    }

    if (!cv::imwrite(job.fileName.toStdString(), source, job.params))
    {
        return {};
    }

    const cv::Mat reloaded = cv::imread(job.fileName.toStdString(), cv::IMREAD_COLOR);
    if (result)
    {
        result->image = reloaded;
        result->fileSize = QFileInfo(job.fileName).size();
    }
    return reloaded;
}

void showWindowAt(const std::string &windowName, const cv::Mat &image, int index)
{
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::imshow(windowName, image);
    cv::resizeWindow(windowName, 420, 320);
    const int columns = 2;
    cv::moveWindow(windowName,
                   40 + (index % columns) * 450,
                   40 + (index / columns) * 360);
}
} // namespace

ImwriteLessonWidget::ImwriteLessonWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(QStringLiteral("imwrite 学习：生成并保存图片"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet(QStringLiteral("color: #555;"));
    statusLabel->setWordWrap(true);

    auto *buttonLayout1 = new QHBoxLayout();
    auto *reloadButton = new QPushButton(QStringLiteral("重新读取 cat.jpg"), this);
    auto *pngButton = new QPushButton(QStringLiteral("PNG 多窗口对比"), this);
    auto *jpegButton = new QPushButton(QStringLiteral("JPEG 多窗口对比"), this);
    buttonLayout1->addStretch();
    buttonLayout1->addWidget(reloadButton);
    buttonLayout1->addWidget(pngButton);
    buttonLayout1->addWidget(jpegButton);
    buttonLayout1->addStretch();

    auto *buttonLayout2 = new QHBoxLayout();
    auto *webpButton = new QPushButton(QStringLiteral("WebP 多窗口对比"), this);
    auto *allButton = new QPushButton(QStringLiteral("全部格式多窗口对比"), this);
    auto *closeWindowsButton = new QPushButton(QStringLiteral("关闭所有对比窗口"), this);
    buttonLayout2->addStretch();
    buttonLayout2->addWidget(webpButton);
    buttonLayout2->addWidget(allButton);
    buttonLayout2->addWidget(closeWindowsButton);
    buttonLayout2->addStretch();

    layout->addWidget(titleLabel);
    layout->addLayout(buttonLayout1);
    layout->addLayout(buttonLayout2);
    layout->addWidget(statusLabel);

    waitKeyTimer = new QTimer(this);
    waitKeyTimer->setInterval(30);
    connect(waitKeyTimer, &QTimer::timeout, this, []() {
        cv::waitKey(1);
    });

    connect(reloadButton, &QPushButton::clicked, this, &ImwriteLessonWidget::loadSourceImage);
    connect(pngButton, &QPushButton::clicked, this, &ImwriteLessonWidget::showPngComparison);
    connect(jpegButton, &QPushButton::clicked, this, &ImwriteLessonWidget::showJpegComparison);
    connect(webpButton, &QPushButton::clicked, this, &ImwriteLessonWidget::showWebpComparison);
    connect(allButton, &QPushButton::clicked, this, &ImwriteLessonWidget::showAllComparisons);
    connect(closeWindowsButton, &QPushButton::clicked, this, &ImwriteLessonWidget::closeAllWindows);

    loadSourceImage();
}

ImwriteLessonWidget::~ImwriteLessonWidget()
{
    closeAllWindows();
}

QString ImwriteLessonWidget::formatFileSize(qint64 bytes)
{
    if (bytes < 1024)
    {
        return QStringLiteral("%1 B").arg(bytes);
    }
    if (bytes < 1024 * 1024)
    {
        return QStringLiteral("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    }

    return QStringLiteral("%1 MB").arg(bytes / 1024.0 / 1024.0, 0, 'f', 2);
}

void ImwriteLessonWidget::loadSourceImage()
{
    sourceImagePath = QStringLiteral("cat.jpg");
    const QString cwd = QDir::currentPath();
    const QString absPath = QFileInfo(sourceImagePath).absoluteFilePath();
    const bool fileExists = QFileInfo::exists(absPath);

    sourceImage = cv::imread(sourceImagePath.toStdString(), cv::IMREAD_COLOR);
    if (sourceImage.empty())
    {
        statusLabel->setText(QStringLiteral("读取失败：%1\n工作目录：%2\n解析路径：%3\n文件存在：%4")
                                 .arg(sourceImagePath)
                                 .arg(cwd)
                                 .arg(absPath)
                                 .arg(fileExists ? QStringLiteral("是") : QStringLiteral("否")));
        return;
    }

    statusLabel->setText(QStringLiteral("已读取：%1\n尺寸：%2 × %3\n工作目录：%4\n现在点击按钮，把这张 cat 图片重新保存成不同格式和质量，再用多个 OpenCV 窗口并排对比。")
                             .arg(sourceImagePath)
                             .arg(sourceImage.cols)
                             .arg(sourceImage.rows)
                             .arg(cwd));
}

void ImwriteLessonWidget::ensureTimer()
{
    if (!waitKeyTimer->isActive())
    {
        waitKeyTimer->start();
    }
}

void ImwriteLessonWidget::closeAllWindows()
{
    for (const auto &windowName : windowNames)
    {
        cv::destroyWindow(windowName);
    }
    windowNames.clear();
}

void ImwriteLessonWidget::showPngComparison()
{
    if (sourceImage.empty())
    {
        loadSourceImage();
        if (sourceImage.empty())
        {
            return;
        }
    }

    closeAllWindows();

    const std::vector<VariantJob> jobs = {
        {QStringLiteral("cat_png_c0.png"), QStringLiteral("PNG compression 0"), {cv::IMWRITE_PNG_COMPRESSION, 0}},
        {QStringLiteral("cat_png_c9.png"), QStringLiteral("PNG compression 9"), {cv::IMWRITE_PNG_COMPRESSION, 9}},
    };

    QStringList lines;
    lines << QStringLiteral("PNG 多窗口对比：原图 + 两种压缩级别。注意 PNG 只改变文件体积，不改变像素。")
          << QStringLiteral("原图：%1").arg(sourceImagePath);

    windowNames.push_back("Original cat.jpg");
    showWindowAt(windowNames.back(), sourceImage, 0);

    int index = 1;
    for (const VariantJob &job : jobs)
    {
        VariantResult result;
        if (saveAndReloadVariant(sourceImage, job, &result).empty())
        {
            lines << QStringLiteral("失败：%1").arg(job.fileName);
            continue;
        }

        const std::string windowName = job.label.toStdString();
        windowNames.push_back(windowName);
        showWindowAt(windowName, result.image, index++);
        lines << QStringLiteral("%1 -> %2").arg(job.label, formatFileSize(result.fileSize));
    }

    statusLabel->setText(lines.join('\n'));
    ensureTimer();
}

void ImwriteLessonWidget::showJpegComparison()
{
    if (sourceImage.empty())
    {
        loadSourceImage();
        if (sourceImage.empty())
        {
            return;
        }
    }

    closeAllWindows();

    const std::vector<VariantJob> jobs = {
        {QStringLiteral("cat_jpeg_q100.jpg"), QStringLiteral("JPEG quality 100"), {cv::IMWRITE_JPEG_QUALITY, 100}},
        {QStringLiteral("cat_jpeg_q40.jpg"), QStringLiteral("JPEG quality 40"), {cv::IMWRITE_JPEG_QUALITY, 40}},
        {QStringLiteral("cat_jpeg_q8.jpg"), QStringLiteral("JPEG quality 8"), {cv::IMWRITE_JPEG_QUALITY, 8}},
    };

    QStringList lines;
    lines << QStringLiteral("JPEG 多窗口对比：这里把质量差距拉到 100 / 40 / 8。质量 8 会明显出现块效应、糊边和细节涂抹。")
          << QStringLiteral("原图：%1").arg(sourceImagePath);

    windowNames.push_back("Original cat.jpg");
    showWindowAt(windowNames.back(), sourceImage, 0);

    int index = 1;
    for (const VariantJob &job : jobs)
    {
        VariantResult result;
        if (saveAndReloadVariant(sourceImage, job, &result).empty())
        {
            lines << QStringLiteral("失败：%1").arg(job.fileName);
            continue;
        }

        const std::string windowName = job.label.toStdString();
        windowNames.push_back(windowName);
        showWindowAt(windowName, result.image, index++);
        lines << QStringLiteral("%1 -> %2").arg(job.label, formatFileSize(result.fileSize));
    }

    statusLabel->setText(lines.join('\n'));
    ensureTimer();
}

void ImwriteLessonWidget::showWebpComparison()
{
    if (sourceImage.empty())
    {
        loadSourceImage();
        if (sourceImage.empty())
        {
            return;
        }
    }

    closeAllWindows();

    const std::vector<VariantJob> jobs = {
        {QStringLiteral("cat_webp_q100.webp"), QStringLiteral("WebP quality 100"), {cv::IMWRITE_WEBP_QUALITY, 100}},
        {QStringLiteral("cat_webp_q35.webp"), QStringLiteral("WebP quality 35"), {cv::IMWRITE_WEBP_QUALITY, 35}},
        {QStringLiteral("cat_webp_q5.webp"), QStringLiteral("WebP quality 5"), {cv::IMWRITE_WEBP_QUALITY, 5}},
    };

    QStringList lines;
    lines << QStringLiteral("WebP 多窗口对比：这里把质量差距拉到 100 / 35 / 5。质量 5 会更容易看出强压缩后的平滑化和细节损失。")
          << QStringLiteral("原图：%1").arg(sourceImagePath);

    windowNames.push_back("Original cat.jpg");
    showWindowAt(windowNames.back(), sourceImage, 0);

    int index = 1;
    for (const VariantJob &job : jobs)
    {
        VariantResult result;
        if (saveAndReloadVariant(sourceImage, job, &result).empty())
        {
            lines << QStringLiteral("失败：%1").arg(job.fileName);
            continue;
        }

        const std::string windowName = job.label.toStdString();
        windowNames.push_back(windowName);
        showWindowAt(windowName, result.image, index++);
        lines << QStringLiteral("%1 -> %2").arg(job.label, formatFileSize(result.fileSize));
    }

    statusLabel->setText(lines.join('\n'));
    ensureTimer();
}

void ImwriteLessonWidget::showAllComparisons()
{
    if (sourceImage.empty())
    {
        loadSourceImage();
        if (sourceImage.empty())
        {
            return;
        }
    }

    closeAllWindows();

    const std::vector<VariantJob> jobs = {
        {QStringLiteral("cat_png_c9.png"), QStringLiteral("PNG compression 9"), {cv::IMWRITE_PNG_COMPRESSION, 9}},
        {QStringLiteral("cat_jpeg_q100.jpg"), QStringLiteral("JPEG quality 100"), {cv::IMWRITE_JPEG_QUALITY, 100}},
        {QStringLiteral("cat_jpeg_q8.jpg"), QStringLiteral("JPEG quality 8"), {cv::IMWRITE_JPEG_QUALITY, 8}},
        {QStringLiteral("cat_webp_q100.webp"), QStringLiteral("WebP quality 100"), {cv::IMWRITE_WEBP_QUALITY, 100}},
        {QStringLiteral("cat_webp_q5.webp"), QStringLiteral("WebP quality 5"), {cv::IMWRITE_WEBP_QUALITY, 5}},
    };

    QStringList lines;
    lines << QStringLiteral("全部格式多窗口对比：使用更极端的 JPEG/WebP 参数，方便直接横向看出压缩差异。")
          << QStringLiteral("原图：%1").arg(sourceImagePath);

    windowNames.push_back("Original cat.jpg");
    showWindowAt(windowNames.back(), sourceImage, 0);

    int index = 1;
    for (const VariantJob &job : jobs)
    {
        VariantResult result;
        if (saveAndReloadVariant(sourceImage, job, &result).empty())
        {
            lines << QStringLiteral("失败：%1").arg(job.fileName);
            continue;
        }

        const std::string windowName = job.label.toStdString();
        windowNames.push_back(windowName);
        showWindowAt(windowName, result.image, index++);
        lines << QStringLiteral("%1 -> %2").arg(job.label, formatFileSize(result.fileSize));
    }

    statusLabel->setText(lines.join('\n'));
    ensureTimer();
}
