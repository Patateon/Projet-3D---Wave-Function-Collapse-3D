#include "grid.h"

Cell::Cell() :  hasMesh(false),object(nullptr){}


Grid::Grid() : resX(0), resY(0), resZ(0) {}


Grid::Grid(int X, int Y, int Z,int dimX,int dimY,int dimZ,QVector3D bbMin,int nModel) {
    cells = std::vector<Cell>(X * Y * Z);
    //std::cout<<cells[0].hasMesh<<std::endl;
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

void Grid::initializeBuffers() {
    initializeOpenGLFunctions();
    size_t totalMatrices = 0;
    for (const auto& matrices : modelMatrixes) {
        totalMatrices += matrices.size();
    }
    glGenBuffers(1, &matrixVBO);
    glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
    glBufferData(GL_ARRAY_BUFFER, totalMatrices * sizeof(QMatrix4x4), nullptr, GL_STATIC_DRAW);
    size_t offset = 0;
    for (unsigned int i = 0; i < modelMatrixes.size(); ++i) {
        const auto& matrices = modelMatrixes[i];
        size_t size = matrices.size() * sizeof(QMatrix4x4);
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, matrices.data());
        offset += size;
    }
    for (unsigned int i = 0; i < modelPos.size(); ++i) {
        if (!modelPos[i].empty()) {
            QOpenGLVertexArrayObject* VAO = modeles[i].mesh().vao;
            VAO->bind();
            for (unsigned int k = 0; k < 4; ++k) {
                glEnableVertexAttribArray(3 + k);
                glVertexAttribPointer(3 + k, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(k * sizeof(QVector4D)));
                glVertexAttribDivisor(3 + k, 1);
            }
            VAO->release();
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Grid::render(GLuint program) {
    glUseProgram(program);
    for (unsigned int i = 0; i < modelPos.size(); ++i) {
        int numInstances = modelPos[i].size();
        if (numInstances > 0) {
            modeles[i].mesh().vao->bind();
            glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
            for (unsigned int k = 0; k < 4; ++k) {
                glEnableVertexAttribArray(3 + k);
                glVertexAttribPointer(3 + k, 4, GL_FLOAT, GL_FALSE, sizeof(QMatrix4x4), (void*)(k * sizeof(QVector4D)));
                glVertexAttribDivisor(3 + k, 1);
            }
            glDrawElementsInstanced(
                GL_TRIANGLES,
                modeles[i].mesh().triangles.size(),
                GL_UNSIGNED_INT,
                0,
                numInstances
                );
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
}

void Grid::setModeles(QVector<TileModel> modeles){
    this->modeles=modeles;
}

void Grid::printGrid()  {
    for (int z = 0; z < resZ; ++z) {
        for (int y = 0; y < resY; ++y) {
            for (int x = 0; x < resX; ++x) {
                const Cell& cell = getCell(x, y, z);
                if (cell.hasMesh) {
                    std::cout << "Cell(" << x << ", " << y << ", " << z << ") hasMesh: " << cell.hasMesh
                              << " mode: " << cell.object.tileModel()->getId() << std::endl;

                }
            }
        }
    }
}



