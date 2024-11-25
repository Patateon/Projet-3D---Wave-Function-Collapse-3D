
#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include "mainviewer.h"
#include "grid.h"



int main( int argc , char** argv )
{
    QApplication app( argc , argv );

    MainViewer * viewer = new MainViewer;

    QMainWindow * mainWindow = new QMainWindow;
    QToolBar * toolBar = new QToolBar;
    toolBar->setIconSize(QSize(35,35));
    toolBar->setAutoFillBackground(true);
    toolBar->setStyleSheet("QToolBar { background: white; }");
    viewer->add_actions_to_toolBar(toolBar);
    mainWindow->addToolBar(toolBar);

    mainWindow->setCentralWidget(viewer);

//    Grid grid(1,1,1,1,1,1,QVector3D(0,0,0),5);

    QObject::connect( viewer , SIGNAL(windowTitleUpdated(QString)) , mainWindow , SLOT(setWindowTitle(QString)) );
    viewer->updateTitle("Wave Function Collapse");

    mainWindow->setWindowIcon(QIcon("img/icons/icon.png"));
    mainWindow->show();

    return app.exec();
}
