
#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include "mainviewer.h"
#include "grid.h"
#include "wfc.h"

void printQSet(const QSet<int>& set) {
    for (const int& element : set) {
        std::cout << element<<std::endl;
    }
}

int main( int argc , char** argv )
{
    QApplication app( argc , argv );

    MainViewer * viewer = new MainViewer;
    viewer->hide();

    QMainWindow * mainWindow = new QMainWindow;
    QToolBar * toolBar = new QToolBar;
    toolBar->setIconSize(QSize(35,35));
    toolBar->setAutoFillBackground(true);
    toolBar->setStyleSheet("QToolBar { background: white; }");
    viewer->add_actions_to_toolBar(toolBar);
    mainWindow->addToolBar(toolBar);

    mainWindow->setCentralWidget(viewer);
    viewer->setMainWindow(mainWindow);

    QObject::connect( viewer , SIGNAL(windowTitleUpdated(QString)) , mainWindow , SLOT(setWindowTitle(QString)) );
    viewer->updateTitle("Wave Function Collapse");

    mainWindow->setWindowIcon(QIcon("img/icons/icon.png"));
    mainWindow->show();

    return app.exec();
}
