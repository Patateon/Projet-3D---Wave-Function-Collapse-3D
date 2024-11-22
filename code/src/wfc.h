#ifndef WFC_H
#define WFC_H
#include <grid.h>


class wfc
{
public:
    wfc(Grid& grid);
    runWFC(Grid& grid,int k,QVector<TileModel> &modeles);
    initWFC(int k,QVector<TileModel> &modeles);
private:
    Grid& m_grid;
};

#endif // WFC_H
