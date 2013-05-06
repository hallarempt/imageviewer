#include "MainWindow.h"
#include <QApplication>
#include <QGLWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setAttribute(Qt::AA_X11InitThreads);
    QGLWidget *rootContext = new QGLWidget();
    MainWindow w(rootContext);
    w.show();
    
    return a.exec();
}
