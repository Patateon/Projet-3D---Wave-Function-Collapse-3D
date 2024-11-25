#include "grid.h"

Cell::Cell() :  hasMesh(false),object(nullptr){}


Grid::Grid() : resX(0), resY(0), resZ(0) {}


Grid::Grid(
    int X, int Y, int Z,
    float dim_x, float dim_y, float dim_z,
    QVector3D bbMin,int nModel)
{
    cells = std::vector<Cell>(X * Y * Z);
    resX = X;
    resY = Y;
    resZ = Z;
    dimX = dim_x;
    dimY = dim_y;
    dimZ = dim_z;
    BBmin = bbMin;
    modelPos.resize(nModel); //redimensionner selon nModel
    for (int i = 0; i < nModel; ++i) {
        modelPos[i].clear();  //Ini vecteur vide
    }
    modelMatrixes.resize(nModel);
    for (int i = 0; i < nModel; ++i) {
        modelMatrixes[i].clear();
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
    cell.object = object;
    cell.hasMesh = true;
    int id = cell.object.tileModel()->getId();
    modelPos[id].push_back(QVector3D(x,y,z));
    //décalage du modèle
    QVector3D size = object.tileModel()->bbmax() - object.tileModel()->bbmin();

    float scaling = fmin(fmin(dimX, dimY), dimZ);
    scaling /= fmax(fmax(size.x(), size.y()), size.z());

    QVector3D scale(scaling, scaling, scaling);
    QVector3D translate(x*dimX, y*dimY, z*dimZ);

    cell.object.transform().scale() = QVector3D(1.0f, 1.0f, 1.0f);
    cell.object.transform().translation() = translate;

    //On met a jour commme ca pour l'instant
    QMatrix4x4 matrix = cell.object.transform().getLocalModel();
    modelMatrixes[id].push_back((matrix));

}

//void Grid::initializeBuffers(QOpenGLShaderProgram* program) {
//    initializeOpenGLFunctions();

//    program->bind();

//    size_t totalMatrices = 0;
//    for (const auto& matrices : modelMatrixes) {
//        totalMatrices += matrices.size();
//    }
//    glGenBuffers(1, &matrixVBO);
//    glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
//    glBufferData(GL_ARRAY_BUFFER,
//                 totalMatrices * sizeof(QMatrix4x4),
//                 nullptr,
//                 GL_STATIC_DRAW);
//    size_t offset = 0;
//    for (int i = 0; i < modelMatrixes.size(); ++i) {
//        const auto& matrices = modelMatrixes[i];
//        size_t size = matrices.size() * sizeof(QMatrix4x4);
//        glBufferSubData(GL_ARRAY_BUFFER, offset, size, matrices.data());
//        offset += size;
//    }
//    for (int i = 0; i < modelPos.size(); ++i) {
//        if (!modelPos[i].empty()) {
//            QOpenGLVertexArrayObject* VAO = modeles[i].mesh().vao;
//            VAO->bind();
//            for (unsigned int k = 0; k < 4; ++k) {
//                glEnableVertexAttribArray(3 + k);
//                glVertexAttribPointer(3 + k, 4,
//                                      GL_FLOAT,
//                                      GL_FALSE,
//                                      sizeof(QMatrix4x4),
//                                      (void*)(k * sizeof(QVector4D)));
//                glVertexAttribDivisor(3 + k, 1);
//            }
//            VAO->release();
//        }
//    }
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    program->release();
//}

void Grid::initializeBuffers(QOpenGLShaderProgram* program) {
    initializeOpenGLFunctions();
    program->bind();

    std::vector<GLuint> matrixVBO(modelPos.size(), 0);

    for(int i = 0; i < modelPos.size(); i++){
        if (modelPos[i].empty()){
            continue;
        }

        size_t totalMatrices = 0;
        for (const auto& matrices : modelMatrixes) {
            totalMatrices += matrices.size();
        }

        glGenBuffers(1, &matrixVBO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, matrixVBO[i]);
        glBufferData(GL_ARRAY_BUFFER,
                     totalMatrices * sizeof(QMatrix4x4),
                     nullptr,
                     GL_STATIC_DRAW);

        size_t offset = 0;
        for (int i = 0; i < modelMatrixes.size(); ++i) {
            const auto& matrices = modelMatrixes[i];
            size_t size = matrices.size() * sizeof(QMatrix4x4);
            glBufferSubData(GL_ARRAY_BUFFER, offset, size, matrices.data());
            offset += size;
        }


        QOpenGLVertexArrayObject* VAO = modeles[i].mesh().vao;
        VAO->bind();
        for (unsigned int k = 0; k < 4; ++k) {
            glEnableVertexAttribArray(3 + k);
            glVertexAttribPointer(3 + k, 4,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  sizeof(QMatrix4x4),
                                  (void*)(k * sizeof(QVector4D)));
            glVertexAttribDivisor(3 + k, 1);
        }
        VAO->release();

    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    program->release();
}


void Grid::render(QOpenGLShaderProgram* program) {
    program->bind();

    for (int i = 0; i < modelPos.size(); ++i) {
        int numInstances = modelPos[i].size();
        if (numInstances > 0) {
            modeles[i].mesh().vao->bind();
            glBindBuffer(GL_ARRAY_BUFFER, matrixVBO[i]);
            for (unsigned int k = 0; k < 4; ++k) {
                glEnableVertexAttribArray(3 + k);

                // TODO : Fixe la segfault et le problème avec les VBO des matrices
//                glVertexAttribPointer(3 + k, 4,
//                                      GL_FLOAT,
//                                      GL_FALSE,
//                                      sizeof(QMatrix4x4),
//                                      (void*)(k * sizeof(QVector4D)));
                glVertexAttribDivisor(3 + k, 1);
            }
            glDrawElementsInstanced(
                GL_TRIANGLES,
                modeles[i].mesh().triangles.size() * 3,
                GL_UNSIGNED_INT,
                0,
                numInstances
                );
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
    program->release();
}

void Grid::setModeles(QVector<TileModel> modeles){
    this->modeles=modeles;
}

void Grid::printGrid()  {
    for (int x = 0; x < resZ; ++x) {
        for (int y = 0; y < resY; ++y) {
            for (int z = 0; z < resX; ++z) {
                Cell& cell = getCell(x, y, z);
                std::cout << "Cell("
                          << x << ", "
                          << y << ", "
                          << z << ") hasMesh: "
                          << cell.hasMesh;

                if (cell.hasMesh){
                    std::cout << " id: "
                              << cell.object.tileModel()->id()
                              << std::endl;
                }else{
                    std::cout << std::endl;
                }
            }
        }
    }
}
