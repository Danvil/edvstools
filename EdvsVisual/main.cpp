#include "WdgtEdvsVisual.h"

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EdvsVisual w;
    w.show();
    return a.exec();
}
