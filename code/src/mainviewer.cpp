#include "mainviewer.h"

MainViewer::~MainViewer(){
    mesh.clear();
    delete program;
}

void MainViewer::add_actions_to_toolBar(QToolBar *toolBar)
{
    // Specify the actions :
    DetailedAction * open_mesh = new DetailedAction( QIcon("./icons/open.png") , "Open Mesh" , "Open Mesh" , this , this , SLOT(open_mesh()) );
    DetailedAction * save_mesh = new DetailedAction( QIcon("./icons/save.png") , "Save model" , "Save model" , this , this , SLOT(save_mesh()) );
    DetailedAction * help = new DetailedAction( QIcon("./icons/help.png") , "HELP" , "HELP" , this , this , SLOT(help()) );
    DetailedAction * saveCamera = new DetailedAction( QIcon("./icons/camera.png") , "Save camera" , "Save camera" , this , this , SLOT(saveCamera()) );
    DetailedAction * openCamera = new DetailedAction( QIcon("./icons/open_camera.png") , "Open camera" , "Open camera" , this , this , SLOT(openCamera()) );
    DetailedAction * saveSnapShotPlusPlus = new DetailedAction( QIcon("./icons/save_snapshot.png") , "Save snapshot" , "Save snapshot" , this , this , SLOT(saveSnapShotPlusPlus()) );

    // Add them :
    toolBar->addAction( open_mesh );
    toolBar->addAction( save_mesh );
    toolBar->addAction( help );
    toolBar->addAction( saveCamera );
    toolBar->addAction( openCamera );
    toolBar->addAction( saveSnapShotPlusPlus );
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
    QString modelPath1 = QString("models/tiles/stair.obj");
    QString modelPath2 = QString("models/tiles/stair.obj");
    QString modelPath3 = QString("models/tiles/slab.obj");
    QString modelPath4 = QString("models/tiles/anti_corner_stair.obj");

    TileModel model1 = TileModel(0, modelPath1);
    TileModel model2 = TileModel(1, modelPath2);
    TileModel model3 = TileModel(2, modelPath3);
    TileModel model4 = TileModel(3, modelPath4);

    QVector<TileModel> modeles;
    modeles.push_back(model1);
    modeles.push_back(model2);
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
    grid->setMode(0);
    std::cout<<"Creation règles"<<std::endl;
    for(int i = 0;i<modeles.size();i++){
        QVector<bool> x_rot={1,static_cast<bool>(i%2),static_cast<bool>((i*3)%2),0};
        QVector<bool> y_rot={1,static_cast<bool>((i+1)%2),static_cast<bool>((i*3)%2),0};
        QVector<bool> z_rot={1,static_cast<bool>(i%2),static_cast<bool>((i*3+1)%2),0};
        QVector<QSet<int>> rules;

        for(int j=0;j<6;j++){
            QSet<int> set;
            set.insert(0);
            set.insert(1);
            //set.insert(2);
            set.insert(3);
            set.insert((j%4+i%4)%4);
            if((j+1)%3<=1){
                set.insert((j%4)%4);
            }

                if((j+1)%3>=1){
                    set.insert((j%3)%4);
                }
                if((j+1)%3<=1){
                    set.insert(((j*3)%4)%4);
                }


            rules.push_back(set);
        }
        modeles[i].setRules(rules[0],rules[1],rules[2],rules[3],rules[4],rules[5]);
        modeles[i].setRots(x_rot,y_rot,z_rot);
        modeles[i].setType(modeles,grid->getMode());

        qDebug() << "Regles modele "<<i;
        qDebug() << "xminus :"<< rules[0];
        qDebug() << "xplus :"<<rules[1];
        qDebug() << "yminus :"<< rules[2];
        qDebug() << "yplus :" <<rules[3];
        qDebug() << "zminus :"<<rules[4];
        qDebug() << "zplus :"<<rules[5];

//        std::cout<<"Regles modele "<<i<<" : "<<std::endl;
//        qDebug() << rules;
    }
    std::cout<<"Fin Creation règles"<<std::endl;


    wfc = new Wfc(*grid);
    std::cout<<"mode : "<<grid->getMode()<<std::endl;;
    wfc->runWFC(5, modeles,grid->getMode());
    grid->printGrid();
    grid->initializeBuffers(program);
}

void MainViewer::initializeProgramShader() {
    initializeOpenGLFunctions();

    initializeShaders();
}

void MainViewer::initializeShaders(){


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

/*
* Lib QGLViewer callback to init, draw and clean
*/

void MainViewer::init() {
    makeCurrent();
    initializeProgramShader();

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

    //
    int boxDimension = 5;
    float boxSpacing = 2.0;
    float boxSize = (float) boxDimension * boxSpacing;

    setSceneCenter( qglviewer::Vec( boxSize / 2.0 , boxSize / 2.0 , boxSize / 2.0 ) );
    setSceneRadius( boxSize );

    //
    initializeBasicWFC(boxDimension, boxSpacing);

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

    camera()->getModelViewMatrix(viewMatrix.data());
    camera()->getProjectionMatrix(projectionMatrix.data());

    QMatrix4x4 model = QMatrix4x4();
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
            point3d bb(FLT_MAX,FLT_MAX,FLT_MAX) , BB(-FLT_MAX,-FLT_MAX,-FLT_MAX);
            for( unsigned int v = 0 ; v < mesh.vertices.size() ; ++v ) {
                bb = point3d::min(bb , mesh.vertices[v]);
                BB = point3d::max(BB , mesh.vertices[v]);
            }
            adjustCamera(bb,BB);
            update();
            mesh.initVAO(program);
        }
        else
            std::cout << fileName.toStdString() << " could not be opened" << std::endl;
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

void MainViewer::saveCameraInFile(const QString &filename){
    std::ofstream out (filename.toUtf8());
    if (!out)
        exit (EXIT_FAILURE);
    // << operator for point3 causes linking problem on windows
    out << camera()->position()[0] << " \t" << camera()->position()[1] << " \t" << camera()->position()[2] << " \t" " " <<
                                      camera()->viewDirection()[0] << " \t" << camera()->viewDirection()[1] << " \t" << camera()->viewDirection()[2] << " \t" << " " <<
                                      camera()->upVector()[0] << " \t" << camera()->upVector()[1] << " \t" <<camera()->upVector()[2] << " \t" <<" " <<
                                      camera()->fieldOfView();
    out << std::endl;

    out.close ();
}

void MainViewer::openCameraFromFile(const QString &filename){

    std::ifstream file;
    file.open(filename.toStdString().c_str());

    qglviewer::Vec pos;
    qglviewer::Vec view;
    qglviewer::Vec up;
    float fov;

    file >> (pos[0]) >> (pos[1]) >> (pos[2])
         >> (view[0]) >> (view[1]) >> (view[2])
         >> (up[0]) >> (up[1]) >> (up[2])
         >> fov;

    camera()->setPosition(pos);
    camera()->setViewDirection(view);
    camera()->setUpVector(up);
    camera()->setFieldOfView(fov);

    camera()->computeModelViewMatrix();
    camera()->computeProjectionMatrix();

    update();
}

void MainViewer::openCamera(){
    QString fileName = QFileDialog::getOpenFileName(NULL,"","*.cam");
    if ( !fileName.isNull() ) {                 // got a file name
        openCameraFromFile(fileName);
    }
}

void MainViewer::saveCamera(){
    QString fileName = QFileDialog::getSaveFileName(NULL,"","*.cam");
    if ( !fileName.isNull() ) {                 // got a file name
        saveCameraInFile(fileName);
    }
}

void MainViewer::saveSnapShotPlusPlus(){
    QString fileName = QFileDialog::getSaveFileName(NULL,"*.png","");
    if ( !fileName.isNull() ) {                 // got a file name
        setSnapshotFormat("PNG");
        setSnapshotQuality(100);
        saveSnapshot( fileName );
        saveCameraInFile( fileName+QString(".cam") );
    }
}
