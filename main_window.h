#pragma once

#include <QMainWindow>

class QStackedWidget;
class QListWidget;
class QPushButton;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *stack = nullptr;
    QListWidget *lessonList = nullptr;
    QPushButton *imwriteBackButton = nullptr;
    QPushButton *imreadBackButton = nullptr;
};
