#include "grid.h"

Cell::Cell() :  hasMesh(false),object(nullptr){}


Grid::Grid() : resX(0), resY(0), resZ(0) {}


Grid::Grid(int X, int Y, int Z,int dimX,int dimY,int dimZ,QVector3D bbMin,int nModel) {
    cells = std::vector<Cell>(X * Y * Z);
    resX = X;
    resY = Y;
    resZ = Z;
    dimX=dimX;
    dimY=dimY;
    dimZ=dimZ;
    BBmin=bbMin;
    modelPos.resize(nModel); //redimensionner selon nModel
    for (int i = 0; i < nModel; ++i) {
        modelPos[i].resize(0);  //Ini vecteur vide
    }
    modelMatrixes.resize(nModel);
    for (int i = 0; i < nModel; ++i) {
        modelMatrixes[i].resize(0);
    }
}


Cell& Grid::getCell(int x, int y, int z) {
    uint index = x + resX * (y + resY * z);
    return cells[index];
}

uint Grid::getCellIndex(int x, int y, int z) const {
    return x + resX * (y + resY * z);
}

void Grid::setObject(TileInstance object,int x,int y,int z){
    Cell& cell = getCell(x,y,z);
    cell.object=object;
    cell.hasMesh = true;
    int id = cell.object.tileModel()->getId();
    modelPos[id].push_back(QVector3D(x,y,z));
    QVector3D translate(x*dimX,y*dimY,z*dimZ);//décalage du modèle
    cell.object.transform().translation() = translate;
    QMatrix4x4 matrix = cell.object.transform().getLocalModel();//On met a jour commme ca pour l'instant
    modelMatrixes[id].push_back((matrix));

}

void Grid::initializeBuffers() {//tous les buffers dans grid pour en avoir qu'un seul et un seul appel a l'affichage? 0 sommet 1 noramle 2 texture 3 4 5 6 matrice transfo
    initializeOpenGLFunctions();
    glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
    glBufferData(GL_ARRAY_BUFFER, modelMatrixes.size() * sizeof(QMatrix4x4), modelMatrixes.data(), GL_STATIC_DRAW);
    for (unsigned int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, 4*sizeof(QVector4D),(void*)(i * sizeof(QVector4D)));
        glVertexAttribDivisor(3 + i, 1);
    }
    glBindVertexArray(0);
}

void Grid::render(GLuint program){
    for(int i = 0;i<modelPos.size();i++){
        for(int j = 0 ; j<modelPos[i].size();j++){
            Cell cell = this->getCell(modelPos[i][j].x(),modelPos[i][j].y(),modelPos[i][j].z());
            if(cell.hasMesh){
                QMatrix4x4 modelMatrix = modelMatrixes[i][j];
                GLuint modelMatrixLocation = glGetUniformLocation(program, "modelMatrix");//Au final revoir pour version instanciée comme sur le learnopengl
                if (modelMatrixLocation != -1) {
                    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, modelMatrix.data());
                }
                //draw
                cell.object.tileModel()->mesh().initVAO(program);
                //cell.object.tileModel()->mesh().render();
            }
        }
    }
}
