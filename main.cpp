#include "spaceteslahunter.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SpaceTeslaHunter w;
    w.show();
    return QApplication::exec();
}
