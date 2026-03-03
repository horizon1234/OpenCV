#include "main_window.h"

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "01 生成并保存图片/imwrite_lesson_widget.h"
#include "02 读取并显示图片/imread_lesson_widget.h"
#include "03 窗口显示/named_window_lesson_widget.h"
#include "04 腐蚀与膨胀/morphology_trackbar_lesson_widget.h"
#include "05 边界提取/erosion_boundary_lesson_widget.h"
#include "06 点运算-灰度变换/point_gray_transform_lesson_widget.h"
#include "07 点运算-直方图/point_histogram_lesson_widget.h"
#include "08 点运算-截断/point_truncation_lesson_widget.h"
#include "09 点运算-提升饱和度与颜色/point_color_adjust_lesson_widget.h"
#include "10 点运算-反相/point_invert_lesson_widget.h"
#include "11 点运算-二值化/point_threshold_lesson_widget.h"
#include "12 点运算-对比度拉伸/point_contrast_stretch_lesson_widget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    stack = new QStackedWidget();

    enum PageIndex
    {
        HomePageIndex = 0,
        ImwritePageIndex = 1,
        ImreadPageIndex = 2,
        NamedWindowPageIndex = 3,
        MorphologyPageIndex = 4,
        ErosionBoundaryPageIndex = 5,
        PointGrayTransformPageIndex = 6,
        PointHistogramPageIndex = 7,
        PointTruncationPageIndex = 8,
        PointColorAdjustPageIndex = 9,
        PointInvertPageIndex = 10,
        PointThresholdPageIndex = 11,
        PointContrastStretchPageIndex = 12
    };

    auto *homePage = new QWidget();
    auto *homeLayout = new QVBoxLayout(homePage);

    auto *homeTitle = new QLabel(QStringLiteral("OpenCV 学习项目"), homePage);
    homeTitle->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    lessonList = new QListWidget(homePage);

    auto *imwriteItem = new QListWidgetItem(QStringLiteral("imwrite：生成并保存图片"));
    imwriteItem->setData(Qt::UserRole, ImwritePageIndex);
    lessonList->addItem(imwriteItem);

    auto *imreadItem = new QListWidgetItem(QStringLiteral("imread：读取并显示图片"));
    imreadItem->setData(Qt::UserRole, ImreadPageIndex);
    lessonList->addItem(imreadItem);

    auto *namedWindowItem = new QListWidgetItem(QStringLiteral("namedWindow：OpenCV 窗口显示"));
    namedWindowItem->setData(Qt::UserRole, NamedWindowPageIndex);
    lessonList->addItem(namedWindowItem);

    auto *morphologyItem = new QListWidgetItem(QStringLiteral("Trackbar：腐蚀与膨胀"));
    morphologyItem->setData(Qt::UserRole, MorphologyPageIndex);
    lessonList->addItem(morphologyItem);

    auto *erosionBoundaryItem = new QListWidgetItem(QStringLiteral("腐蚀应用：边界提取"));
    erosionBoundaryItem->setData(Qt::UserRole, ErosionBoundaryPageIndex);
    lessonList->addItem(erosionBoundaryItem);

    auto *pointGrayItem = new QListWidgetItem(QStringLiteral("点运算：灰度变换"));
    pointGrayItem->setData(Qt::UserRole, PointGrayTransformPageIndex);
    lessonList->addItem(pointGrayItem);

    auto *pointHistogramItem = new QListWidgetItem(QStringLiteral("点运算：直方图均衡化"));
    pointHistogramItem->setData(Qt::UserRole, PointHistogramPageIndex);
    lessonList->addItem(pointHistogramItem);

    auto *pointTruncationItem = new QListWidgetItem(QStringLiteral("点运算：截断"));
    pointTruncationItem->setData(Qt::UserRole, PointTruncationPageIndex);
    lessonList->addItem(pointTruncationItem);

    auto *pointColorItem = new QListWidgetItem(QStringLiteral("点运算：提升饱和度/颜色增强"));
    pointColorItem->setData(Qt::UserRole, PointColorAdjustPageIndex);
    lessonList->addItem(pointColorItem);

    auto *pointInvertItem = new QListWidgetItem(QStringLiteral("点运算：反相"));
    pointInvertItem->setData(Qt::UserRole, PointInvertPageIndex);
    lessonList->addItem(pointInvertItem);

    auto *pointThresholdItem = new QListWidgetItem(QStringLiteral("点运算：二值化"));
    pointThresholdItem->setData(Qt::UserRole, PointThresholdPageIndex);
    lessonList->addItem(pointThresholdItem);

    auto *pointContrastItem = new QListWidgetItem(QStringLiteral("点运算：对比度拉伸"));
    pointContrastItem->setData(Qt::UserRole, PointContrastStretchPageIndex);
    lessonList->addItem(pointContrastItem);

    homeLayout->addWidget(homeTitle);
    homeLayout->addWidget(lessonList, 1);

    auto *imwritePage = new QWidget();
    auto *imwriteLayout = new QVBoxLayout(imwritePage);
    imwriteBackButton = new QPushButton(QStringLiteral("返回首页"), imwritePage);
    auto *imwriteLesson = new ImwriteLessonWidget(imwritePage);

    imwriteLayout->addWidget(imwriteBackButton, 0, Qt::AlignLeft);
    imwriteLayout->addWidget(imwriteLesson, 1);

    auto *imreadPage = new QWidget();
    auto *imreadLayout = new QVBoxLayout(imreadPage);
    imreadBackButton = new QPushButton(QStringLiteral("返回首页"), imreadPage);
    auto *imreadLesson = new ImreadLessonWidget(imreadPage);

    imreadLayout->addWidget(imreadBackButton, 0, Qt::AlignLeft);
    imreadLayout->addWidget(imreadLesson, 1);

    auto *namedWindowPage = new QWidget();
    auto *namedWindowLayout = new QVBoxLayout(namedWindowPage);
    auto *namedWindowBackButton = new QPushButton(QStringLiteral("返回首页"), namedWindowPage);
    auto *namedWindowLesson = new NamedWindowLessonWidget(namedWindowPage);

    namedWindowLayout->addWidget(namedWindowBackButton, 0, Qt::AlignLeft);
    namedWindowLayout->addWidget(namedWindowLesson, 1);

    auto *morphologyPage = new QWidget();
    auto *morphologyLayout = new QVBoxLayout(morphologyPage);
    auto *morphologyBackButton = new QPushButton(QStringLiteral("返回首页"), morphologyPage);
    auto *morphologyLesson = new MorphologyTrackbarLessonWidget(morphologyPage);

    morphologyLayout->addWidget(morphologyBackButton, 0, Qt::AlignLeft);
    morphologyLayout->addWidget(morphologyLesson, 1);

    auto *erosionBoundaryPage = new QWidget();
    auto *erosionBoundaryLayout = new QVBoxLayout(erosionBoundaryPage);
    auto *erosionBoundaryBackButton = new QPushButton(QStringLiteral("返回首页"), erosionBoundaryPage);
    auto *erosionBoundaryLesson = new ErosionBoundaryLessonWidget(erosionBoundaryPage);

    erosionBoundaryLayout->addWidget(erosionBoundaryBackButton, 0, Qt::AlignLeft);
    erosionBoundaryLayout->addWidget(erosionBoundaryLesson, 1);

    auto *pointGrayPage = new QWidget();
    auto *pointGrayLayout = new QVBoxLayout(pointGrayPage);
    auto *pointGrayBackButton = new QPushButton(QStringLiteral("返回首页"), pointGrayPage);
    auto *pointGrayLesson = new PointGrayTransformLessonWidget(pointGrayPage);

    pointGrayLayout->addWidget(pointGrayBackButton, 0, Qt::AlignLeft);
    pointGrayLayout->addWidget(pointGrayLesson, 1);

    auto *pointHistogramPage = new QWidget();
    auto *pointHistogramLayout = new QVBoxLayout(pointHistogramPage);
    auto *pointHistogramBackButton = new QPushButton(QStringLiteral("返回首页"), pointHistogramPage);
    auto *pointHistogramLesson = new PointHistogramLessonWidget(pointHistogramPage);

    pointHistogramLayout->addWidget(pointHistogramBackButton, 0, Qt::AlignLeft);
    pointHistogramLayout->addWidget(pointHistogramLesson, 1);

    auto *pointTruncationPage = new QWidget();
    auto *pointTruncationLayout = new QVBoxLayout(pointTruncationPage);
    auto *pointTruncationBackButton = new QPushButton(QStringLiteral("返回首页"), pointTruncationPage);
    auto *pointTruncationLesson = new PointTruncationLessonWidget(pointTruncationPage);

    pointTruncationLayout->addWidget(pointTruncationBackButton, 0, Qt::AlignLeft);
    pointTruncationLayout->addWidget(pointTruncationLesson, 1);

    auto *pointColorPage = new QWidget();
    auto *pointColorLayout = new QVBoxLayout(pointColorPage);
    auto *pointColorBackButton = new QPushButton(QStringLiteral("返回首页"), pointColorPage);
    auto *pointColorLesson = new PointColorAdjustLessonWidget(pointColorPage);

    pointColorLayout->addWidget(pointColorBackButton, 0, Qt::AlignLeft);
    pointColorLayout->addWidget(pointColorLesson, 1);

    auto *pointInvertPage = new QWidget();
    auto *pointInvertLayout = new QVBoxLayout(pointInvertPage);
    auto *pointInvertBackButton = new QPushButton(QStringLiteral("返回首页"), pointInvertPage);
    auto *pointInvertLesson = new PointInvertLessonWidget(pointInvertPage);

    pointInvertLayout->addWidget(pointInvertBackButton, 0, Qt::AlignLeft);
    pointInvertLayout->addWidget(pointInvertLesson, 1);

    auto *pointThresholdPage = new QWidget();
    auto *pointThresholdLayout = new QVBoxLayout(pointThresholdPage);
    auto *pointThresholdBackButton = new QPushButton(QStringLiteral("返回首页"), pointThresholdPage);
    auto *pointThresholdLesson = new PointThresholdLessonWidget(pointThresholdPage);

    pointThresholdLayout->addWidget(pointThresholdBackButton, 0, Qt::AlignLeft);
    pointThresholdLayout->addWidget(pointThresholdLesson, 1);

    auto *pointContrastPage = new QWidget();
    auto *pointContrastLayout = new QVBoxLayout(pointContrastPage);
    auto *pointContrastBackButton = new QPushButton(QStringLiteral("返回首页"), pointContrastPage);
    auto *pointContrastLesson = new PointContrastStretchLessonWidget(pointContrastPage);

    pointContrastLayout->addWidget(pointContrastBackButton, 0, Qt::AlignLeft);
    pointContrastLayout->addWidget(pointContrastLesson, 1);

    stack->addWidget(homePage);
    stack->addWidget(imwritePage);
    stack->addWidget(imreadPage);
    stack->addWidget(namedWindowPage);
    stack->addWidget(morphologyPage);
    stack->addWidget(erosionBoundaryPage);
    stack->addWidget(pointGrayPage);
    stack->addWidget(pointHistogramPage);
    stack->addWidget(pointTruncationPage);
    stack->addWidget(pointColorPage);
    stack->addWidget(pointInvertPage);
    stack->addWidget(pointThresholdPage);
    stack->addWidget(pointContrastPage);

    QObject::connect(lessonList, &QListWidget::itemClicked, stack, [this](QListWidgetItem *item) {
        const int pageIndex = item->data(Qt::UserRole).toInt();
        stack->setCurrentIndex(pageIndex);
    });
    QObject::connect(imwriteBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });
    QObject::connect(imreadBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });
    QObject::connect(namedWindowBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });
    QObject::connect(morphologyBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });
    QObject::connect(erosionBoundaryBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });
    QObject::connect(pointGrayBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });
    QObject::connect(pointHistogramBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });
    QObject::connect(pointTruncationBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });
    QObject::connect(pointColorBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });
    QObject::connect(pointInvertBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });
    QObject::connect(pointThresholdBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });
    QObject::connect(pointContrastBackButton, &QPushButton::clicked, stack, [this]() {
        stack->setCurrentIndex(HomePageIndex);
    });

    setWindowTitle(QStringLiteral("Qt + OpenCV 学习项目"));
    setCentralWidget(stack);
    resize(800, 600);
}
