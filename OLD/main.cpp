#include "gui/testerwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TesterWindow w;
    w.show();
    return a.exec();
}
