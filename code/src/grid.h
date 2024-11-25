#ifndef GRID_H
#define GRID_H

#include <QVector3D>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <vector>
#include <tileinstance.h>

struct Cell {
    bool hasMesh;
    TileInstance object;

    Cell();
};

class Grid : protected QOpenGLFunctions_4_5_Core {
public:
    Grid();
    Grid(int X, int Y, int Z, float dim_x, float dim_y, float dim_z, QVector3D bbMin,int nModel);

    Cell& getCell(int x, int y, int z);
    void setObject(TileInstance object, int x, int y, int z);

    uint getCellIndex(int x, int y, int z) const;
    void initializeBuffers(QOpenGLShaderProgram* program);
    void render(QOpenGLShaderProgram* program);
    void setModeles(QVector<TileModel> modeles);

    void printGrid();

private:
    QVector3D BBmin;
    QVector3D BBmax;
    int resX;
    int resY;
    int resZ;
    std::vector<Cell> cells;
    float dimX;
    float dimY;
    float dimZ;
    QVector<QVector<QVector3D>> modelPos;
    QVector<QVector<QMatrix4x4>> modelMatrixes;
    QVector<GLuint> matrixVBO;

    // TODO : Remplacer par un set
    QVector<TileModel> modeles;//Charger les modeles a mettre dans le MyViewer?
};

#endif // GRID_H
