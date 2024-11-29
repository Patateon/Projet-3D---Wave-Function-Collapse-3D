#ifndef WFC_H
#define WFC_H
#include <grid.h>
#include <QVector>

class Wfc
{
public:
    Wfc();
    Wfc(Grid& grid);
    void runWFC(Grid& grid,int k,QVector<TileModel> &modeles);
    void initWFC(int k,QVector<TileModel> &modeles);
    bool ruleCheck(QVector<QSet<int>> voisins,int id_center);
private:
    Grid& m_grid;
};

#endif // WFC_H
