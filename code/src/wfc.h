#ifndef WFC_H
#define WFC_H
#include <grid.h>
#include <QVector>

class Wfc
{
public:
    Wfc();
    Wfc(Grid& grid);
    void runWFC(int k,QVector<TileModel> &modeles,int mode);
    void initWFC(int k,QVector<TileModel> &modeles,int mode);
    bool ruleCheck(QVector<QSet<int>> voisins,int id_center);
    QSet<int> getCorrespondingRules(QVector3D pos,QVector3D posV,TileModel *model);
private:
    Grid& m_grid;
};

#endif // WFC_H
