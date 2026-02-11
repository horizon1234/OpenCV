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
        ErosionBoundaryPageIndex = 5
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

    stack->addWidget(homePage);
    stack->addWidget(imwritePage);
    stack->addWidget(imreadPage);
    stack->addWidget(namedWindowPage);
    stack->addWidget(morphologyPage);
    stack->addWidget(erosionBoundaryPage);

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

    setWindowTitle(QStringLiteral("Qt + OpenCV 学习项目"));
    setCentralWidget(stack);
    resize(800, 600);
}
