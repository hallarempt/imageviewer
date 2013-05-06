#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGLWidget>

class DisplayWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QGLWidget *context, QWidget *parent = 0);
    ~MainWindow();
    
private slots:

    void slotQuit();
    void slotOpen();

private:
    Ui::MainWindow *ui;

    DisplayWidget *m_displayWidget;
};

#endif // MAINWINDOW_H
