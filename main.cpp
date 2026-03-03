#include <QApplication>
#include <QDir>

#include "main_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QDir::setCurrent(QApplication::applicationDirPath());

    MainWindow window;
    window.show();

    return app.exec();
}
