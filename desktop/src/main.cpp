#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("OpenBand");
    QCoreApplication::setOrganizationName("JcDesigns");

    MainWindow window;
    window.show();
    return app.exec();
}
