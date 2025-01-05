#include "grid.h"


Cell::Cell()
    : hasMesh(false), object(nullptr)
{}

Grid::Grid()
    : resX(0),resY(0), resZ(0), linesInitialized(false)
{
    selectCell(0, 0, 0);
}

Grid::Grid(
    int X, int Y, int Z,
    float dim_x, float dim_y, float dim_z,
    QVector3D bbMin, int nModel)
{
    cells = std::vector<Cell>(X * Y * Z);
    resX = X;
    resY = Y;
    resZ = Z;
    dimX = dim_x;
    dimY = dim_y;
    dimZ = dim_z;
    BBmin = bbMin;
    selectCell(0, 0, 0);
    modelPos.resize(nModel); // Redimensionner selon nModel
    for (int i = 0; i < nModel; ++i) {
        modelPos[i].clear();  // Ini vecteur vide
    }
    modelMatrixes.resize(nModel);
    for (int i = 0; i < nModel; ++i) {
        modelMatrixes[i].clear();
    }
}

Grid::~Grid() {
    // Nettoyage du VBO
    clean();
}

Cell& Grid::getCell(int x, int y, int z) {
    uint index = x + resX * (y + resY * z);
    return cells[index];
}

uint Grid::getCellIndex(int x, int y, int z) const {
    return x + resX * (y + resY * z);
}

void Grid::getCoordinates(uint index, int& x, int& y, int& z) const {
    z = index / (resX * resY);
    uint remainder2D = index % (resX * resY);
    y = remainder2D / resX;
    x = remainder2D % resX;
}

void Grid::deleteInstance(uint x, uint y, uint z){
    Cell& cell = getCell(x, y, z);

    if (!cell.hasMesh) {
        return;
    }

    int id = cell.object.tileModel()->id();
    modelPos[id].removeOne(QVector3D(x, y, z));
    modelMatrixes[id].removeOne(cell.object.transform().getLocalModel());

    cell.object = nullptr;
    cell.hasMesh = false;
}

void Grid::setObject(TileInstance object, int x, int y, int z, float x_deg, float y_deg, float z_deg) {
    Cell& cell = getCell(x, y, z);
    cell.object = object;
    cell.hasMesh = true;
    int id = cell.object.tileModel()->id();
    modelPos[id].push_back(QVector3D(x, y, z));
    // Décalage du modèle
    QVector3D size = object.tileModel()->bbmax() - object.tileModel()->bbmin();

    float scaling = fmin(fmin(dimX, dimY), dimZ);
    scaling /= fmax(fmax(size.x(), size.y()), size.z());

    QVector3D scale(scaling, scaling, scaling);
    QVector3D translate(x * dimX, y * dimY, z * dimZ);
    translate = BBmin + translate + QVector3D(dimX/2, dimY/2, dimZ/2);
    QQuaternion rotation = QQuaternion::fromEulerAngles(QVector3D(x_deg, y_deg, z_deg));

    cell.object.transform().scale() = scale;
    cell.object.transform().translation() = translate;
    cell.object.transform().rotation() = rotation;

    // On met à jour comme ça pour l'instant
    QMatrix4x4 matrix = cell.object.transform().getLocalModel();
    modelMatrixes[id].push_back(matrix);
}

void Grid::initializeBuffers(QOpenGLShaderProgram* program) {
    if (modelPos.size() != modelMatrixes.size()) {
        qWarning() << "Error: modelPos and modelMatrixes size mismatch!";
        return;
    }

    initializeOpenGLFunctions();
    // Optionally, clear the matrixVBOs if they need to be re-initialized later
    for (auto& vbo : matrixVBO) {
        if (vbo != 0) {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
    }

    program->bind();

    matrixVBO.clear();
    matrixVBO = QVector<GLuint>(modelPos.size(), 0);

    for (int i = 0; i < modelPos.size(); i++) {
        if (modelPos[i].empty()) {
            continue;
        }

        const auto& currentMatrix = modelMatrixes[i];

        glGenBuffers(1, &matrixVBO[i]);

        glBindBuffer(GL_ARRAY_BUFFER, matrixVBO[i]);
        glBufferData(GL_ARRAY_BUFFER,
                     currentMatrix.size() * sizeof(QMatrix4x4),
                     currentMatrix.data(),
                     GL_STATIC_DRAW);

        QOpenGLVertexArrayObject* VAO = modeles[i]->mesh().vao;
        if (VAO && VAO->isCreated()) {
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
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    program->release();
    m_gridInitialized = true;
}

void Grid::render(QOpenGLShaderProgram* program) {

    if (!m_gridInitialized){
        return;
    }

    program->bind();
    for (int i = 0; i < modelPos.size(); ++i) {
        int numInstances = modelPos[i].size();

        if (numInstances > 0) {

            if (!modeles[i]->mesh().vao->isCreated()) {
                qWarning() << "Invalid VAO for model" << i;
                continue;
            }

            modeles[i]->mesh().vao->bind();
            if (matrixVBO[i] != 0) {
                glBindBuffer(GL_ARRAY_BUFFER, matrixVBO[i]);
            } else {
                continue;
            }

            for (unsigned int k = 0; k < 4; ++k) {
                glEnableVertexAttribArray(3 + k);
                glVertexAttribPointer(3 + k,
                                      4,
                                      GL_FLOAT,
                                      GL_FALSE,
                                      sizeof(QMatrix4x4),
                                      (void*)(k * sizeof(QVector4D)));
                glVertexAttribDivisor(3 + k, 1);
            }

            glDrawElementsInstanced(
                GL_TRIANGLES,
                modeles[i]->mesh().triangles.size() * 3,
                GL_UNSIGNED_INT,
                0,
                numInstances
                );

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            modeles[i]->mesh().vao->release();
        }
    }
    program->release();
    drawCell();
}

void Grid::setModeles(QVector<TileModel*> modeles) {
    this->modeles = modeles;
}


QVector<TileModel*> Grid::getModeles(){
    return modeles;
}

void Grid::drawNormales(QOpenGLShaderProgram* program){
    for(int i = 0; i < modelPos.size(); i++){
        int numInstances = modelPos[i].size();
        if (numInstances > 0) {
            modeles[i]->mesh().vao->bind();
            if (matrixVBO[i] != 0) {
                glBindBuffer(GL_ARRAY_BUFFER, matrixVBO[i]);
            } else {
                continue;
            }

            modeles[i]->mesh().renderVAONormalLine(program);
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
}

void Grid::printGrid() {
    for (int z = 0; z < resZ; ++z) {
        for (int y = 0; y < resY; ++y) {
            for (int x = 0; x < resX; ++x) {
                const Cell& cell = getCell(x, y, z);
                if (cell.hasMesh) {
                    std::cout << "Cell(" << x
                              << ", " << y
                              << ", " << z << ") hasMesh: "
                              << cell.hasMesh
                              << " model: " << cell.object.tileModel()->id()
                              << " entropie : "<<cell.entropy
                    << " type :"<<cell.object.tileModel()->getType()<< std::endl;
                }
            }
        }
    }
}

// Selection functions
void Grid::selectCell(uint x, uint y, uint z)
{
    selectedCell = getCellIndex(x, y, z);
    selectedCellCoord = QVector3D(x*dimX, y*dimY, z*dimZ) + BBmin;
}

void Grid::unselectCell()
{
    selectedCell = -1;
}

void Grid::moveSelection(int axis, int step){

    int x, y, z;
    getCoordinates(selectedCell, x, y, z);

    if (axis == 0)
    {
        x = std::max(0, std::min(resX-1, x + step));
    }
    else if (axis == 1)
    {
        y = std::max(0, std::min(resY-1, y + step));
    }
    else if (axis == 2)
    {
        z = std::max(0, std::min(resZ-1, z + step));
    }
    else
    {
        qWarning("Wrong axis selected: Should be 0 (X), 1 (Y) or 2(Z)");
        return;
    }

    selectCell(x, y, z);
}

void Grid::rotateSelection(QOpenGLShaderProgram* program, int axis, int step){    int x, y, z;
    getCoordinates(selectedCell, x, y, z);

    Cell cell = getCell(x, y, z);
    if (!cell.hasMesh){
        return;
    }
    QQuaternion incrementRotation;

    if (axis == 0)
        incrementRotation = QQuaternion::fromEulerAngles(step * 90, 0, 0);
    else if (axis == 1)
        incrementRotation = QQuaternion::fromEulerAngles(0, step * 90, 0);
    else if (axis == 2)
        incrementRotation = QQuaternion::fromEulerAngles(0, 0, step * 90);
    else {
        qWarning("Wrong axis selected: Should be 0 (X), 1 (Y) or 2(Z)");
        return;
    }

    QQuaternion rotation = cell.object.transform().rotation();
    QQuaternion newRotation = rotation * incrementRotation;

    float pitch, yaw, roll;
    newRotation.getEulerAngles(&pitch, &yaw, &roll);

    deleteInstance(x, y, z);
    setObject(cell.object, x, y, z, pitch, yaw, roll);

    initializeBuffers(program);
}

void Grid::displayCell(bool showGrid){
    m_showSelectedCell = showGrid;
}

void Grid::cleanTransform(){

}

void Grid::drawCell()
{
    // Force à moi, TODO
    if (selectedCell == -1 || !m_showSelectedCell){
        return;
    }
    initializeOpenGLFunctions();

    // Affichage de la cellule actuellement selectionnée avec du
    // legacy OpenGL
    glDisable(GL_LIGHTING);
    glLineWidth(5.0f);

    QVector3D p0 = selectedCellCoord;
    QVector3D p1 = selectedCellCoord + QVector3D(dimX, 0.0f, 0.0f);
    QVector3D p2 = selectedCellCoord + QVector3D(dimX, 0.0f, dimZ);
    QVector3D p3 = selectedCellCoord + QVector3D(0.0f, 0.0f, dimZ);
    QVector3D p4 = selectedCellCoord + QVector3D(0.0f, dimY, 0.0f);
    QVector3D p5 = selectedCellCoord + QVector3D(dimX, dimY, 0.0f);
    QVector3D p6 = selectedCellCoord + QVector3D(dimX, dimY, dimZ);
    QVector3D p7 = selectedCellCoord + QVector3D(0.0f, dimY, dimZ);

    glBegin(GL_LINES);

    glColor3f(1.0f, 0.5f, 0.0f);

    // Affichage des lignes de la cellule sélectionnée
    glVertex3f(p0.x(), p0.y(), p0.z());
    glVertex3f(p1.x(), p1.y(), p1.z());
    glVertex3f(p1.x(), p1.y(), p1.z());
    glVertex3f(p2.x(), p2.y(), p2.z());
    glVertex3f(p2.x(), p2.y(), p2.z());
    glVertex3f(p3.x(), p3.y(), p3.z());
    glVertex3f(p3.x(), p3.y(), p3.z());
    glVertex3f(p0.x(), p0.y(), p0.z());

    glVertex3f(p4.x(), p4.y(), p4.z());
    glVertex3f(p5.x(), p5.y(), p5.z());
    glVertex3f(p5.x(), p5.y(), p5.z());
    glVertex3f(p6.x(), p6.y(), p6.z());
    glVertex3f(p6.x(), p6.y(), p6.z());
    glVertex3f(p7.x(), p7.y(), p7.z());
    glVertex3f(p7.x(), p7.y(), p7.z());
    glVertex3f(p4.x(), p4.y(), p4.z());

    glVertex3f(p0.x(), p0.y(), p0.z());
    glVertex3f(p4.x(), p4.y(), p4.z());
    glVertex3f(p1.x(), p1.y(), p1.z());
    glVertex3f(p5.x(), p5.y(), p5.z());
    glVertex3f(p2.x(), p2.y(), p2.z());
    glVertex3f(p6.x(), p6.y(), p6.z());
    glVertex3f(p3.x(), p3.y(), p3.z());
    glVertex3f(p7.x(), p7.y(), p7.z());

    glEnd();

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

void Grid::clean() {
    // Iterate through all cells and reset them
    for (int z = 0; z < resZ; ++z) {
        for (int y = 0; y < resY; ++y) {
            for (int x = 0; x < resX; ++x) {
                Cell& cell = getCell(x, y, z);
                cell.hasMesh = false;
                cell.object = nullptr;
                cell.entropy = 0.0f; // Assuming entropy should be reset too
            }
        }
    }

    // Clear the model position and matrix vectors
    for (auto& posVec : modelPos) {
        posVec.clear();
    }

    for (auto& matrixVec : modelMatrixes) {
        matrixVec.clear();
    }

    // Optionally, clear the matrixVBOs if they need to be re-initialized later
    for (auto& vbo : matrixVBO) {
        if (vbo != 0) {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
    }

    lineVBO.destroy();
}

QVector3D Grid::getCellCoordinates(int x, int y, int z) {
    // Calcul des coordonnées en fonction de la taille des cellules et de leur position dans la grille
    float coordX = x * dimX + dimX / 2.0f;
    float coordY = y * dimY + dimY / 2.0f;
    float coordZ = z * dimZ + dimZ / 2.0f;

    // Retourne les coordonnées sous forme de QVector3D
    return QVector3D(coordX, coordY, coordZ);
}

bool Grid::isTypeClose(int x, int y, int z, uint type) {
    for (int dz = -1; dz <= 1; ++dz) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0 && dz == 0) {
                    continue;
                }
                int nx = x + dx;
                int ny = y + dy;
                int nz = z + dz;
                if (nx >= 0 && nx < resX && ny >= 0 && ny < resY && nz >= 0 && nz < resZ) {
                    const Cell& cell = getCell(nx, ny, nz);
                    if (cell.hasMesh && cell.object.tileModel()->id() == type) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void removeElementFromVec(QVector<QVector3D>& vec, const QVector3D& toRemove) {
    for(int i=0;i<3;i++){
        int index = vec.indexOf(toRemove);
        if (index != -1)
            vec.removeAt(index);
    }
}

bool contains(QVector<QVector3D>& vec, const QVector3D& value) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

int getAxisSign(QVector3D vec){
    if(vec.x()==-1) return 0;
    if(vec.x()==1) return 1;
    if(vec.y()==-1) return 2;
    if(vec.y()==1) return 3;
    if(vec.z()==-1) return 4;
    if(vec.z()==1) return 5;
}
QVector3D approxAngle(QVector3D &angle) {
    float epsilon = 5.0f;
    float tolerance = 0.1f;
    if (fabs(angle.x() + 90.0f) < tolerance) angle.setX(270.0f);
    if (fabs(angle.y() + 90.0f) < tolerance) angle.setY(270.0f);
    if (fabs(angle.z() + 90.0f) < tolerance) angle.setZ(270.0f);
    for (int i = 0; i < 4; i++) {

        if (angle.x() > (i * 90) - epsilon && angle.x() < (i * 90) + epsilon) {
            angle.setX(i * 90);
        }
        if (angle.y() > (i * 90) - epsilon && angle.y() < (i * 90) + epsilon) {
            angle.setY(i * 90);
        }
        if (angle.z() > (i * 90) - epsilon && angle.z() < (i * 90) + epsilon) {
            angle.setZ(i * 90);
        }
    }
    return angle;
}


QVector<TileModel*> Grid::createRules(){//Créer les règles a partir d'une grille contenant juste des tilemodels sans regles
    //Regles de type sont automatiques selon le nombre de voisins par rapport a la taille de modeles
    QVector<TileModel*> models = this->getModeles();
    QVector<QVector<QSet<int>>> rulesT(models.size(), QVector<QSet<int>>(6));
    QVector<QVector<QVector<bool>>> rotT(models.size(), QVector<QVector<bool>>(3, QVector<bool>(4, false)));
    for(int x = 0;x<resX;x++){
        for(int y = 0;y<resY;y++){
            for(int z = 0;z<resZ;z++){
                if(getCell(x,y,z).hasMesh){
                    QVector<QVector3D> voisins;
                    voisins.push_back(QVector3D(std::min(x+1,resX-1),y,z));
                    if(!contains(voisins,QVector3D(std::max(x-1,0),y,z))) voisins.push_back(QVector3D(std::max(x-1,0),y,z));
                    if(!contains(voisins,QVector3D(x,std::min(y+1,resY-1),z)))voisins.push_back(QVector3D(x,std::min(y+1,resY-1),z));
                    if(!contains(voisins,QVector3D(x,std::max(y-1,0),z)))voisins.push_back(QVector3D(x,std::max(y-1,0),z));
                    if(!contains(voisins,QVector3D(x,y,std::min(z+1,resZ-1))))voisins.push_back(QVector3D(x,y,std::min(z+1,resZ-1)));
                    if(!contains(voisins,QVector3D(x,y,std::max(z-1,0))))voisins.push_back(QVector3D(x,y,std::max(z-1,0)));
                    removeElementFromVec(voisins,QVector3D(x,y,z));
                    int id=getCell(x,y,z).object.tileModel()->id();
                    //Récupération rotation
                    Transform transform =getCell(x,y,z).object.transform();
                    QVector3D angles = transform.getRotationAngles();
                    angles=approxAngle(angles);
                    int indexX=std::abs(angles[0]/90.0f);
                    int indexY=std::abs(angles[1]/90.0f);
                    int indexZ=std::abs(angles[2]/90.0f);
                    qDebug()<<angles;
                    if(indexX!=0){
                        rotT[id][0][indexX]=1;
                    }
                    if(indexY!=0){
                        rotT[id][1][indexY]=1;
                    }
                    if(indexZ!=0){
                        rotT[id][2][indexZ]=1;
                    }
                    if(mode==0){
                        QVector<QSet<int>> rules(6);//regles du modeles en cours id
                        for(int i=0;i<voisins.size();i++){
                            QVector3D pos=voisins[i];
                            pos=pos-QVector3D(x,y,z);
                            if(getCell(voisins[i].x(),voisins[i].y(),voisins[i].z()).hasMesh)
                                rules[getAxisSign(pos)].insert(getCell(voisins[i].x(),voisins[i].y(),voisins[i].z()).object.tileModel()->id());
                        }
                        for(int i=0;i<6;i++){
                            for (int model : rules[i]) {
                                rulesT[id][i].insert(model);
                            }
                        }
                    }
                    else{
                        QSet<int> rules;
                        for(int i = 0;i<voisins.size();i++){
                            if(getCell(voisins[i].x(),voisins[i].y(),voisins[i].z()).hasMesh){
                                rules.insert(getCell(voisins[i].x(),voisins[i].y(),voisins[i].z()).object.tileModel()->id());
                            }
                        }
                        for (int model : rules) {
                            for (int i = 0; i < 6; i++) {
                                rulesT[id][i].insert(model);
                            }
                        }
                    }

                }
            }
        }
    }
    for(int id=0;id<models.size();id++){
        //rotation de base a 0 degré autorisée quoi qu'il arrive
        rotT[id][0][0]=1;rotT[id][1][0]=1;rotT[id][2][0]=1;
    }
    for(int i=0;i<models.size();i++){
        models[i]->setRules(rulesT[i][0],rulesT[i][1],rulesT[i][2],rulesT[i][3],rulesT[i][4],rulesT[i][5]);
        models[i]->setRots(rotT[i][0],rotT[i][1],rotT[i][2]);
    }
    for(int i = 0;i<rulesT.size();i++){
        qDebug()<<"REgle modele "<<i;
        qDebug()<<rulesT[i];
        qDebug()<<rotT[i];
    }
    return models;
}


int Grid::getMode(){
    return mode;
}

void Grid::setMode(int m){
    mode = m;
}

bool Grid::isInGrid(int x, int y, int z) const {
    return x >= 0 && x < resX && y >= 0 && y < resY && z >= 0 && z < resZ;
}

void Grid::initGridLines(QOpenGLShaderProgram* program) {
    gridLines.clear();

    // Lignes parallèles à l'axe X
    for (int z = 0; z <= resZ; ++z) {
        for (int y = 0; y <= resY; ++y) {
            gridLines.push_back(QVector3D(0.0f, y * dimY, z * dimZ) + BBmin);
            gridLines.push_back(QVector3D(resX * dimX, y * dimY, z * dimZ) + BBmin);
        }
    }

    // Lignes parallèles à l'axe Y
    for (int z = 0; z <= resZ; ++z) {
        for (int x = 0; x <= resX; ++x) {
            gridLines.push_back(QVector3D(x * dimX, 0.0f, z * dimZ) + BBmin);
            gridLines.push_back(QVector3D(x * dimX, resY * dimY, z * dimZ) + BBmin);
        }
    }

    // Lignes parallèles à l'axe Z
    for (int y = 0; y <= resY; ++y) {
        for (int x = 0; x <= resX; ++x) {
            gridLines.push_back(QVector3D(x * dimX, y * dimY, 0.0f) + BBmin);
            gridLines.push_back(QVector3D(x * dimX, y * dimY, resZ * dimZ) + BBmin);
        }
    }

    // Génération et allocation d'un VBO pour les lignes de la grille
    if(!lineVBO.create()){
        qWarning() << "Could not create grid line VBO!";
        return;
    }

    program->bind();
    lineVBO.bind();

    lineVBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
    lineVBO.allocate(gridLines.data(), gridLines.size() * sizeof(QVector3D));

    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
    program->enableAttributeArray(0);

    lineVBO.release();
    program->release();
}

void Grid::drawGridLines(QOpenGLShaderProgram* program) {

    if(!lineVBO.isCreated()){
        return;
    }
    initializeOpenGLFunctions();

    QVector3D color(0.5f, 0.5f, 0.5f); // Gris
    program->bind();

    // Envoie de la couleur au fragment shader
    program->setUniformValue("color", color);

    lineVBO.bind();
    // Affichage des lignes de la grille
    glDrawArrays(GL_LINES, 0, gridLines.size());

    lineVBO.release();
    program->release();
}
