
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

    QMainWindow * mainWindow = new QMainWindow;
    QToolBar * toolBar = new QToolBar;
    toolBar->setIconSize(QSize(35,35));
    toolBar->setAutoFillBackground(true);
    toolBar->setStyleSheet("QToolBar { background: white; }");
    viewer->add_actions_to_toolBar(toolBar);
    mainWindow->addToolBar(toolBar);

    mainWindow->setCentralWidget(viewer);

    Grid grid(3,3,3,1,1,1,QVector3D(0,0,0),3);
    QVector<TileModel> modeles;
    for(int i = 0;i<3;i++){
        QSet<int> rules;
        rules.insert(std::min(i+1,2));
        if(i!=1){
            rules.insert(std::max(i-1,0));
        }
        TileModel tile(i);
        tile.setRules(rules);
        modeles.push_back(tile);
        std::cout<<"Regles modele "<<i<<" : "<<std::endl;
        printQSet( rules);
    }

    Wfc wfc(grid);
    wfc.runWFC(grid,8,modeles);

    QObject::connect( viewer , SIGNAL(windowTitleUpdated(QString)) , mainWindow , SLOT(setWindowTitle(QString)) );
    viewer->updateTitle("Wave Function Collapse");

    mainWindow->setWindowIcon(QIcon("img/icons/icon.png"));
    mainWindow->show();

    return app.exec();
}
