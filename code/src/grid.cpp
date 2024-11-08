#include "grid.h"


Cell::Cell() :  hasMesh(false), meshIndex(-1) {}


Grid::Grid() : resX(0), resY(0), resZ(0) {}


Grid::Grid(int X, int Y, int Z,QVector3D bbMin) {
    cells = std::vector<Cell>(X * Y * Z);
    resX = X;
    resY = Y;
    resZ = Z;
    BBMin=bbMin;
}


Cell& Grid::getCell(int x, int y, int z) {
    uint index = x + resX * (y + resY * z);
    return cells[index];
}

uint Grid::getCellIndex(int x, int y, int z) const {
    return x + resX * (y + resY * z);
}

void Grid::setMesh(GameObject gameObject,int x,int y,int z){
    Cell& index = getCell(x,y,z);
    mesh=gameObject
}
