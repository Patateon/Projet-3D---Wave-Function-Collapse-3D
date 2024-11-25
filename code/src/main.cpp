
#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include "MyViewer.h"
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

    MyViewer * viewer = new MyViewer;

    QMainWindow * mainWindow = new QMainWindow;
    QToolBar * toolBar = new QToolBar;
    toolBar->setIconSize(QSize(35,35));
    toolBar->setAutoFillBackground(true);
    toolBar->setStyleSheet("QToolBar { background: white; }");
    viewer->add_actions_to_toolBar(toolBar);
    mainWindow->addToolBar(toolBar);

    mainWindow->setCentralWidget(viewer);

    Grid grid(3,3,2,1,1,1,QVector3D(0,0,0),2);
    QVector<TileModel> modeles;
    for(int i = 0;i<2;i++){
        QSet<int> rules;
        rules.insert(std::min(i+1,1));
        TileModel tile(i);
        tile.setRules(rules);
        modeles.push_back(tile);
        std::cout<<"Regles modele "<<i<<" : "<<std::endl;
        printQSet( rules);
    }

    Wfc wfc(grid);
    wfc.runWFC(grid,5,modeles);
    grid.printGrid();

    QObject::connect( viewer , SIGNAL(windowTitleUpdated(QString)) , mainWindow , SLOT(setWindowTitle(QString)) );
    viewer->updateTitle("Wave Function Collapse");

    mainWindow->setWindowIcon(QIcon("img/icons/icon.png"));
    mainWindow->show();

    return app.exec();
}
