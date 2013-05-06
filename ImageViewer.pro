#-------------------------------------------------
#
# Project created by QtCreator 2013-04-23T15:03:42
#
#-------------------------------------------------

QT += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageViewer
TEMPLATE = app

SOURCES += main.cpp\
        MainWindow.cpp \
    DisplayWidget.cpp \
    Node.cpp

HEADERS  += MainWindow.h \
    DisplayWidget.h \
    Node.h

FORMS    += MainWindow.ui

RESOURCES += shaders.qrc

OTHER_FILES += gl2.frag \
    gl2.vert
