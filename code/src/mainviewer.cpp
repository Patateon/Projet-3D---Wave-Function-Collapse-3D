#include "mainviewer.h"

MainViewer::MainViewer(QGLWidget * parent) : QGLViewer(parent), QOpenGLFunctions_4_3_Core() {
    // Vérifiez que vous avez un layout défini
    if (!layout()) {
        setLayout(new QVBoxLayout());  // Ajoutez un layout pour gérer les widgets
    }
}
MainViewer::~MainViewer(){
    mesh.clear();
    delete program;
}

void MainViewer::add_actions_to_toolBar(QToolBar *toolBar)
{
    // Specify the actions :
    DetailedAction * open_mesh = new DetailedAction( QIcon("./icons/open.png") , "Open Mesh" , "Open Mesh" , this , this , SLOT(open_mesh()) );
    DetailedAction * create_init_grid = new DetailedAction( QIcon("./icons/rubik.png") , "Create initialization grid" , "Create initialization grid" , this , this , SLOT(create_initialization_grid()) );
    // Add them :
    toolBar->addAction( open_mesh );
    toolBar->addAction( create_init_grid);
}

void MainViewer::pickBackgroundColor() {
    QColor _bc = QColorDialog::getColor( this->backgroundColor(), this);
    if( _bc.isValid() ) {
        this->setBackgroundColor( _bc );
        this->update();
    }
}

void MainViewer::adjustCamera( point3d const & bb , point3d const & BB ) {
    point3d const & center = ( bb + BB )/2.f;
    setSceneCenter( qglviewer::Vec( center[0] , center[1] , center[2] ) );
    setSceneRadius( 1.5f * ( BB - bb ).norm() );
    showEntireScene();
}

/*
* Initialize routines
*/

void MainViewer::initializeModels() {

}

void MainViewer::initializeRandomGrid(uint dimension, float spacing) {

    QString modelPath1 = QString("models/sphere.off");
    QString modelPath2 = QString("models/arma.off");
    QString modelPath3 = QString("models/monkey.off");

    TileModel model1 = TileModel(0, modelPath1);
    TileModel model2 = TileModel(1, modelPath1);
    TileModel model3 = TileModel(2, modelPath1);
    TileModel model4 = TileModel(3, modelPath1);

    QVector<TileModel> modeles;
    modeles.append(model1);
    modeles.append(model2);
    modeles.append(model3);
    modeles.append(model4);

    for(int i = 0; i < modeles.size(); i++){
        modeles[i].mesh().initVAO(program);
    }

    grid = new Grid(dimension, dimension, dimension,
                    spacing, spacing, spacing,
                    QVector3D(), 4);
    grid->setModeles(modeles);

    for (uint x = 0; x < dimension; x++){
        for (uint y = 0; y < dimension; y++){
            for (uint z = 0; z < dimension; z++){
                int model = rand()%2;
                TileInstance instance = TileInstance(&modeles[model]);
                grid->setObject(instance, x, y, z, 0, 0, 0);
            }
        }
    }

    grid->initializeBuffers(program);
}

void MainViewer::initializeBasicWFC(uint dimension, float spacing) {
    QString modelPath1 = QString("models/tiles/empty.obj");
    QString modelPath2 = QString("models/tiles/stair.obj");
    QString modelPath3 = QString("models/tiles/slab.obj");
    QString modelPath4 = QString("models/monkey.off");

    TileModel model1 = TileModel(0, modelPath1);
    TileModel model2 = TileModel(1, modelPath2);
    TileModel model3 = TileModel(2, modelPath3);
    TileModel model4 = TileModel(3, modelPath4);

    QVector<TileModel> modeles;
    modeles.append(model1);
    modeles.append(model2);
    modeles.append(model3);
    modeles.append(model4);
  
    QVector<QVector<bool>> x_rot;
    QVector<QVector<bool>> y_rot;
    QVector<QVector<bool>> z_rot;
  
    for(int i = 0; i < modeles.size(); i++){
        modeles[i].mesh().initVAO(program);
    }

    grid = new Grid(dimension, dimension, dimension,
                    spacing, spacing, spacing,
                    QVector3D(spacing/2.0, spacing/2.0, spacing/2.0), 4);
    grid->setModeles(modeles);

    for(int i = 0;i<4;i++){
        QSet<int> rules;
        QVector<bool> x_rot={1,static_cast<bool>(i%2),static_cast<bool>((i*3)%2),0};
        QVector<bool> y_rot={1,static_cast<bool>((i+1)%2),static_cast<bool>((i*3)%2),0};
        QVector<bool> z_rot={1,static_cast<bool>(i%2),static_cast<bool>((i*3+1)%2),0};
        rules.insert(std::min(i+1,2));
        if(i!=1){
            rules.insert(std::max(i-1,0));
        }
        modeles[i].setRules(rules);
        modeles[i].setRots(x_rot,y_rot,z_rot);
        modeles[i].setType(modeles);
//        std::cout<<"Regles modele "<<i<<" : "<<std::endl;
//        qDebug() << rules;
        std::cout<<"type : "<<modeles[i].getType()<<std::endl;
    }

    wfc = new Wfc(*grid);
    wfc->runWFC(20, modeles);

    grid->initializeBuffers(program);
}

void MainViewer::initGrid(uint dimension, float spacing){
    grid = new Grid(dimension, dimension, dimension,
                    spacing, spacing, spacing,
                    QVector3D(spacing/2.0, spacing/2.0, spacing/2.0), 4);
    grid->setModeles(m_modeles);

}

void MainViewer::initializeShaders() {
    initializeOpenGLFunctions();

    initializeProgramShader();
    initializeGridLineShader();
}

void MainViewer::initializeProgramShader(){


    delete program;
    program = new QOpenGLShaderProgram(this);
    std::string path = "shaders/";
    std::string vShaderPath = path + "vshader.glsl";
    std::string fShaderPath = path + "fshader.glsl";

    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vShaderPath.c_str())){
        qWarning() << "Erreur de compilation du vertex shader :" << program->log();
    }

    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, fShaderPath.c_str())){
        qWarning() << "Erreur de compilation du fragment shader :" << program->log();
    }

    if (!program->link()){
        qWarning() << "Erreur de lors de la liaison des shaders :" << program->log();
    }
}

void MainViewer::initializeGridLineShader(){
    delete gridLineShader;
    gridLineShader = new QOpenGLShaderProgram(this);

    std::string path = "shaders/";
    std::string vShaderPath = path + "lineVShader.glsl";
    std::string fShaderPath = path + "lineFShader.glsl";

    if (!gridLineShader->addShaderFromSourceFile(QOpenGLShader::Vertex, vShaderPath.c_str())){
        qWarning() << "Erreur de compilation du line vertex shader :" << gridLineShader->log();
    }

    if (!gridLineShader->addShaderFromSourceFile(QOpenGLShader::Fragment, fShaderPath.c_str())){
        qWarning() << "Erreur de compilation du line fragment shader :" << gridLineShader->log();
    }

    if (!gridLineShader->link()){
        qWarning() << "Erreur de lors de la liaison des line shaders :" << gridLineShader->log();
    }
}

/*
* Lib QGLViewer callback to init, draw and clean
*/

void MainViewer::init() {
    makeCurrent();
    initializeShaders();

    setMouseTracking(true);// Needed for MouseGrabber.

    setBackgroundColor(QColor(255,255,255));

    // Lights:
    GLTools::initLights();
    GLTools::setSunsetLight();
    GLTools::setDefaultMaterial();

    //
    glShadeModel(GL_SMOOTH);
    glFrontFace(GL_CCW); // CCW ou CW

    glEnable(GL_DEPTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CLIP_PLANE0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_COLOR_MATERIAL);

    //initializeBasicWFC(3, 1);
    grid->initGridLines(program);

    showEntireScene();

}

void MainViewer::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_wired) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    QMatrix4x4 viewMatrix;
    QMatrix4x4 projectionMatrix;
    QMatrix4x4 model = QMatrix4x4();

    camera()->getModelViewMatrix(viewMatrix.data());
    camera()->getProjectionMatrix(projectionMatrix.data());

    QMatrix4x4 viewProjection = projectionMatrix * viewMatrix; // OpenGL : Projection * View
    qglviewer::Vec qglCameraPosition = camera()->position();
    QVector3D cameraPosition = QVector3D(qglCameraPosition.x,
                                         qglCameraPosition.y,
                                         qglCameraPosition.z);

    drawAxis(5);

    program->bind();
    program->setUniformValue("cameraPosition", cameraPosition);
    program->setUniformValue("viewProjMatrix", viewProjection);

    glEnable(GL_DEPTH_TEST);
    grid->render(program);

    program->release();

    if (m_showgrid){
        gridLineShader->bind();

        gridLineShader->setUniformValue("projectionMatrix", projectionMatrix);  // Matrice de projection
        gridLineShader->setUniformValue("viewMatrix", viewMatrix);              // Matrice de vue
        gridLineShader->setUniformValue("modelMatrix", model);

        grid->drawGridLines(gridLineShader);

        gridLineShader->release();
    }
}

/*
* Others
*/

QString MainViewer::helpString() const {
    QString text("<h2>Our cool project</h2>");
    text += "<p>";
    text += "This is a research application, it can explode.";
    text += "<h3>Participants</h3>";
    text += "<ul>";
    text += "<li>...</li>";
    text += "</ul>";
    text += "<h3>Basics</h3>";
    text += "<p>";
    text += "<ul>";
    text += "<li>H   :   make this help appear</li>";
    text += "<li>Ctrl + mouse right button double click   :   choose background color</li>";
    text += "<li>Ctrl + T   :   change window title</li>";
    text += "</ul>";
    return text;
}

void MainViewer::updateTitle( QString text ) {
    this->setWindowTitle( text );
    emit windowTitleUpdated(text);
}

/*
* Event handlers
*/

void MainViewer::keyPressEvent( QKeyEvent * event ) {
    if( event->key() == Qt::Key_G) {
        m_showgrid = !m_showgrid;
    }
    if( event->key() == Qt::Key_W) {
        m_wired = !m_wired;
    }
    if( event->key() == Qt::Key_H ) {
        help();
    }
    else if( event->key() == Qt::Key_T ) {
        if( event->modifiers() & Qt::CTRL )
        {
            bool ok;
            QString text = QInputDialog::getText(this, tr(""), tr("title:"), QLineEdit::Normal,this->windowTitle(), &ok);
            if (ok && !text.isEmpty())
            {
                updateTitle(text);
            }
        }
    }
}

void MainViewer::mouseDoubleClickEvent( QMouseEvent * e )
{
    if( (e->modifiers() & Qt::ControlModifier)  &&  (e->button() == Qt::RightButton) )
    {
        pickBackgroundColor();
        return;
    }

    if( (e->modifiers() & Qt::ControlModifier)  &&  (e->button() == Qt::LeftButton) )
    {
        showControls();
        return;
    }

    QGLViewer::mouseDoubleClickEvent( e );
}

void MainViewer::mousePressEvent(QMouseEvent* e ) {
    QGLViewer::mousePressEvent(e);
}

void MainViewer::mouseMoveEvent(QMouseEvent* e  ){
    QGLViewer::mouseMoveEvent(e);
}

void MainViewer::mouseReleaseEvent(QMouseEvent* e  ) {
    QGLViewer::mouseReleaseEvent(e);
}

/*
* Slots
*/

void MainViewer::open_mesh() {
    bool success = false;
    QString fileName = QFileDialog::getOpenFileName(NULL,"","");
    if ( !fileName.isNull() ) { // got a file name
        if(fileName.endsWith(QString(".off")))
            success = OFFIO::openTriMesh(fileName.toStdString() , mesh.vertices , mesh.triangles );
        else if(fileName.endsWith(QString(".obj")))
            success = OBJIO::openTriMesh(fileName.toStdString() , mesh.vertices , mesh.triangles );
        if(success) {
            std::cout << fileName.toStdString() << " was opened successfully" << std::endl;
            TileModel modele(m_modeles.size(),fileName);
            m_modeles.push_back(modele);
            for(int i = 0 ;i<m_modeles.size();i++){
                qDebug()<<m_modeles[i].mesh().vertices.size();
            }
            /*
            point3d bb(FLT_MAX,FLT_MAX,FLT_MAX) , BB(-FLT_MAX,-FLT_MAX,-FLT_MAX);
            for( unsigned int v = 0 ; v < mesh.vertices.size() ; ++v ) {
                bb = point3d::min(bb , mesh.vertices[v]);
                BB = point3d::max(BB , mesh.vertices[v]);
            }
            adjustCamera(bb,BB);
            update();
            mesh.initVAO(program);*/
        }
        else
            std::cout << fileName.toStdString() << " could not be opened" << std::endl;
    }
}

void MainViewer::create_initialization_grid() {
    bool ok;
    QInputDialog *inputDialog = new QInputDialog(this);
    inputDialog->setLabelText(tr("Entrer la résolution de la grille :"));
    inputDialog->setTextValue(QString::number(0));
    inputDialog->setDoubleRange(-10000, 10000);
    inputDialog->setDoubleDecimals(2);
    inputDialog->resize(400, 200);
    double value1 = inputDialog->getDouble(this, tr("Résolution de la grille"), tr("Résolution :"), 0, 0, 1000, 2, &ok);
    if (ok) {
        inputDialog->setLabelText(tr("Entrer la taille des cellules :"));
        double value2 = inputDialog->getDouble(this, tr("Taille des cellules"), tr("Taille de cellule :"), 0, 0, 100, 2, &ok);
        if (ok) {
            initGrid(value1, value2);
            float boxSize = (float)value1 * value2;
            setSceneCenter(qglviewer::Vec(boxSize / 2.0, boxSize / 2.0, boxSize / 2.0));
            setSceneRadius(boxSize);
            show();
            QDialog *dialog = new QDialog(this);
            dialog->setWindowTitle("Liste des modèles");
            dialog->setModal(false);
            QVBoxLayout *layout = new QVBoxLayout(dialog);

            modelList = new QListWidget(dialog); // Assignez à la variable membre
            modelList->setMinimumWidth(200);
            modelList->setMinimumHeight(200);

            for (int i = 0; i < m_modeles.size(); ++i) {
                modelList->addItem(m_modeles[i].getName());
            }
            connect(modelList, &QListWidget::itemDoubleClicked, this, &MainViewer::onModelDoubleClicked);
            layout->addWidget(modelList);
            dialog->resize(300, 300);
            dialog->show();
        }
    }
    delete inputDialog;
}

void MainViewer::onModelDoubleClicked(QListWidgetItem *item) {
    if (!m_mainWindow) {
        qWarning() << "m_mainWindow is not initialized!";
        return;
    }

    // Trouver l'index du modèle correspondant à l'item cliqué
    int modelIndex = modelList->row(item);
    if (modelIndex == -1) {
        qWarning() << "Model not found!";
        return;
    }

    // Créer une nouvelle fenêtre (QDialog) pour afficher les boutons
    QDialog *dialog = new QDialog(m_mainWindow);
    dialog->setWindowTitle("Sélectionner les orientations possibles : ");
    dialog->setModal(true);  // Optionnel : mettre le dialogue en modal (bloque l'interface principale)

    // Créer un layout pour les boutons
    QGridLayout *gridLayout = new QGridLayout(dialog);

    // Liste des orientations
    QList<QPair<QString, int>> orientations = {
        {"x", 90}, {"x", 180}, {"x", 270},
        {"y", 90}, {"y", 180}, {"y", 270},
        {"z", 90}, {"z", 180}, {"z", 270}
    };

    // Préparer les vecteurs de rotation initiaux
    TileModel &model = m_modeles[modelIndex];
    QVector<bool> rotx = model.getXRot();  // Supposez que vous avez des getters appropriés
    QVector<bool> roty = model.getYRot();
    QVector<bool> rotz = model.getZRot();

    // Créer les boutons et les connecter au slot
    for (int i = 0; i < orientations.size(); ++i) {
        QString axis = orientations[i].first;
        int angle = orientations[i].second;
        QString buttonText = QString("%1: %2").arg(axis).arg(angle);
        QPushButton *button = new QPushButton(buttonText, dialog);
        connect(button, &QPushButton::clicked, this, [=]() {
            onOrientationButtonClicked(modelIndex, axis, angle);
        });
        gridLayout->addWidget(button, i / 3, i % 3);

        // Mettre à jour la couleur initiale du bouton
        updateButtonColor(button, axis, angle, rotx, roty, rotz);
    }

    dialog->resize(300, 200);
    QRect screenGeometry = QApplication::desktop()->availableGeometry();
    int x = (screenGeometry.width() - dialog->width()) / 2;
    int y = (screenGeometry.height() - dialog->height()) / 2;
    dialog->move(x, y);
    dialog->show();
}

void MainViewer::onOrientationButtonClicked(int modelIndex, const QString &axis, int angle) {
    // Accéder au modèle correspondant
    TileModel &model = m_modeles[modelIndex];

    // Préparer les vecteurs de rotation
    QVector<bool> rotx = model.getXRot();
    QVector<bool> roty = model.getYRot();
    QVector<bool> rotz = model.getZRot();

    // Appliquer la rotation choisie
    if (axis == "x") {
        if (angle == 90) rotx[0] = !rotx[0];
        else if (angle == 180) rotx[1] = !rotx[1];
        else if (angle == 270) rotx[2] = !rotx[2];
    } else if (axis == "y") {
        if (angle == 90) roty[0] = !roty[0];
        else if (angle == 180) roty[1] = !roty[1];
        else if (angle == 270) roty[2] = !roty[2];
    } else if (axis == "z") {
        if (angle == 90) rotz[0] = !rotz[0];
        else if (angle == 180) rotz[1] = !rotz[1];
        else if (angle == 270) rotz[2] = !rotz[2];
    }

    // Utiliser la méthode setRots pour mettre à jour les rotations
    model.setRots(rotx, roty, rotz);

    // Mettre à jour la couleur des boutons
    updateButtonColors(rotx, roty, rotz);
    QApplication::processEvents();
    QWidget *parentWidget = this->parentWidget(); // Si MainViewer est dans une autre fenêtre, parentWidget va pointer sur le parent
    if (parentWidget) {
        parentWidget->repaint();  // Redessin complet de la fenêtre parente (avec les boutons mis à jour)
    }
}

void MainViewer::updateButtonColor(QPushButton *button, const QString &axis, int angle, const QVector<bool> &rotx, const QVector<bool> &roty, const QVector<bool> &rotz) {
    bool isActive = false;
    if (axis == "x") {
        if (angle == 90) isActive = rotx[0];
        else if (angle == 180) isActive = rotx[1];
        else if (angle == 270) isActive = rotx[2];
    } else if (axis == "y") {
        if (angle == 90) isActive = roty[0];
        else if (angle == 180) isActive = roty[1];
        else if (angle == 270) isActive = roty[2];
    } else if (axis == "z") {
        if (angle == 90) isActive = rotz[0];
        else if (angle == 180) isActive = rotz[1];
        else if (angle == 270) isActive = rotz[2];
    }

    // Modifier la couleur du bouton en fonction de l'état de la rotation
    if (isActive) {
        button->setStyleSheet("background-color: green");
    } else {
        button->setStyleSheet("");  // Réinitialiser la couleur de fond
    }

    QTimer::singleShot(0, button, SLOT(update()));
}

void MainViewer::updateButtonColors(const QVector<bool> &rotx, const QVector<bool> &roty, const QVector<bool> &rotz) {
    // Récupérer tous les boutons dans le dialog et les mettre à jour
    QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        QString text = button->text();
        QStringList parts = text.split(": ");
        if (parts.size() == 2) {
            QString axis = parts[0];
            int angle = parts[1].toInt();
            updateButtonColor(button, axis, angle, rotx, roty, rotz);
        }
    }
}




void MainViewer::save_mesh() {
    bool success = false;
    QString fileName = QFileDialog::getOpenFileName(NULL,"","");
    if ( !fileName.isNull() ) { // got a file name
        if(fileName.endsWith(QString(".off")))
            success = OFFIO::save(fileName.toStdString() , mesh.vertices , mesh.triangles );
        else if(fileName.endsWith(QString(".obj")))
            success = OBJIO::save(fileName.toStdString() , mesh.vertices , mesh.triangles );
        if(success)
            std::cout << fileName.toStdString() << " was saved" << std::endl;
        else
            std::cout << fileName.toStdString() << " could not be saved" << std::endl;
    }
}

void MainViewer::showControls()
{
    // Show controls :
    controls->close();
    controls->show();
}



/*
 * Nécessaires pour load les modeles pour ini
 */

QVector<TileModel> MainViewer::getModeles(){
    return m_modeles;
}

void MainViewer::setModeles(QVector<TileModel> modeles){
    m_modeles=modeles;
}

void MainViewer::setMainWindow(QMainWindow *mainWindow) {
    m_mainWindow = mainWindow;
}
