#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QImage>
#include <QFileDialog>
#include <DisplayWidget.h>

MainWindow::MainWindow(QGLWidget *context, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionOpen, SIGNAL(activated()), SLOT(slotOpen()));
    connect(ui->actionQuit, SIGNAL(activated()), SLOT(slotQuit()));

    m_displayWidget = new DisplayWidget(context, this);
    setCentralWidget(m_displayWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotQuit()
{
    qApp->closeAllWindows();
}

void MainWindow::slotOpen()
{
    QStringList fns = QFileDialog::getOpenFileNames(this,
                                                    "Select one or more files to open",
                                                    "/home/boud/bla/data",
                                                    "Images (*.png *.xpm *.jpg)");
    foreach(const QString & fn, fns) {
        QImage img;
        if (QFile::exists(fn)) {
            img.load(fn);
        }
        if (!img.isNull()) {
            img = img.convertToFormat(QImage::Format_ARGB32);
            m_displayWidget->addLayer(img);
        }
    }
    m_displayWidget->updateProjection();
}
