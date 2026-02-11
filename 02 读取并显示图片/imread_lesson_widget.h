#pragma once

#include <QWidget>
#include <QImage>
#include <QString>

class QLabel;

class ImreadLessonWidget : public QWidget
{
public:
    explicit ImreadLessonWidget(QWidget *parent = nullptr);

private:
    QLabel *titleLabel = nullptr;
    QLabel *imageLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QImage correctImage;
    QImage wrongStepImage;
    QString statusText;

    void loadAndShowImage();
};
