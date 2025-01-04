#ifndef GRID_H
#define GRID_H

#include <QVector3D>
#include <QVector>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <vector>
#include <tileinstance.h>
#include <QMatrix4x4>

struct Cell {
    bool hasMesh;
    TileInstance object;
    int entropy = 0;
    Cell();
};

class Grid : protected QOpenGLFunctions_4_5_Core {
public:
    Grid();
    Grid(int X, int Y, int Z, float dim_x, float dim_y, float dim_z, QVector3D bbMin, int nModel);
    ~Grid();

    // Getters and Setters
    int getX() { return resX; }
    int getY() { return resY; }
    int getZ() { return resZ; }
    Cell& getCell(int x, int y, int z);
    uint getCellIndex(int x, int y, int z) const;
    void getCoordinates(uint index, int& x, int& y, int& z) const;
    QVector3D getCellCoordinates(int x, int y, int z);
    void setObject(TileInstance object, int x, int y, int z);
    void setObject(TileInstance object, int x, int y, int z, float x_rot, float y_rot, float z_rot);
    void deleteInstance(uint x, uint y, uint z);
    void setModeles(QVector<TileModel*> modeles);
    QVector<TileModel*> getModeles();

    // Function to selected a particular select and display it
    void selectCell(uint x, uint y, uint z);
    void unselectCell();
    void drawCell();
    bool isDisplayingCell() {return m_showSelectedCell;}
    void displayCell(bool showGrid);
    // Function to move the selected Cell
    void moveSelection(int axis, int step);
    void rotateSelection(QOpenGLShaderProgram *program, int axis, int step);
    int selectedCellIdx() {return selectedCell;}

    // Clean matrix transform
    void cleanTransform();

    // Draw Functions
    void initializeBuffers(QOpenGLShaderProgram* program);
    void render(QOpenGLShaderProgram* program);
    void clean();

    bool isTypeClose(int x, int y, int z, uint type);
    QVector<TileModel*> createRules();
  
    bool isInGrid(int x, int y, int z) const;

    // Debug Functions
    void drawNormales(QOpenGLShaderProgram* program);
    void printGrid();

    int getMode();
    void setMode(int mode);
    void initGridLines(QOpenGLShaderProgram* program);
    void drawGridLines(QOpenGLShaderProgram* program);
  
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
  
    QVector<TileModel*> modeles; // Charger les modeles a mettre dans le MyViewer?
    int mode;//Notion de voisinage simple ou précis

    int selectedCell = -1; // Indice de la cellule actuellement sélectionner, -1 si pas sélectionner
    QVector3D selectedCellCoord;
    bool m_showSelectedCell = true;

    std::vector<QVector3D> gridLines; // Lignes de la grille
    QOpenGLBuffer lineVBO; // VBO pour les lignes de la grille
    bool m_gridInitialized = false;
    bool linesInitialized; // Indique si les lignes de la grille ont été initialisées
};

#endif // GRID_H
