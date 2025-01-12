#include "basicviewer.h"

void BasicViewer::initializeShader() {
    delete m_program;
    m_program = new QOpenGLShaderProgram(this);
    std::string path = "shaders/";
    std::string vShaderPath = path + "basicViewer.vert";
    std::string fShaderPath = path + "fshader.glsl";

    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, vShaderPath.c_str())){
        qWarning() << "Erreur de compilation du vertex shader :" << m_program->log();
    }

    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, fShaderPath.c_str())){
        qWarning() << "Erreur de compilation du fragment shader :" << m_program->log();
    }

    if (!m_program->link()){
        qWarning() << "Erreur de lors de la liaison des shaders :" << m_program->log();
    }

}

void BasicViewer::adjustCamera(const QVector3D &bbmin, const QVector3D &bbmax){
    QVector3D const & center = ( bbmin + bbmax )/2.f;
    setSceneCenter( qglviewer::Vec( center[0] , center[1] , center[2] ) );
    setSceneRadius( 0.5f * ( bbmax - bbmin ).length() );
    showEntireScene();
}

void BasicViewer::init() {
    initializeOpenGLFunctions();
    initializeShader();

    if (m_hasModel) {
        m_tilemodel->mesh().initVAO(m_program);
        adjustCamera(m_tilemodel->bbmin(), m_tilemodel->bbmax());
    }

    setMouseTracking(true);
    setBackgroundColor(QColor(255, 255, 255));

    m_isInitialized = true;
}

void BasicViewer::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_hasModel){
        return;
    }

    if (!context()->isValid()) {
        qWarning() << "Contexte OpenGL invalide dans BasicViewer::draw()";
        return;
    }

    QMatrix4x4 viewMatrix;
    QMatrix4x4 projectionMatrix;
    QMatrix4x4 model = QMatrix4x4();

    camera()->getModelViewMatrix(viewMatrix.data());
    camera()->getProjectionMatrix(projectionMatrix.data());

    QMatrix4x4 viewProjection = projectionMatrix * viewMatrix;
    qglviewer::Vec qglCameraPosition = camera()->position();
    QVector3D cameraPosition = QVector3D(qglCameraPosition.x,
                                         qglCameraPosition.y,
                                         qglCameraPosition.z);

    m_program->bind();
    m_program->setUniformValue("cameraPosition", cameraPosition);
    m_program->setUniformValue("viewProjMatrix", viewProjection);
    m_program->setUniformValue("modelMatrix", model);

    glEnable(GL_DEPTH_TEST);
    m_tilemodel->mesh().render(m_program);

    m_program->release();
}

void BasicViewer::setTileModel(TileModel *tileModel) {
    this->m_tilemodel = tileModel;

    if (m_isInitialized) {
        makeCurrent();
        m_tilemodel->mesh().initVAO(m_program);
        adjustCamera(m_tilemodel->bbmin(), m_tilemodel->bbmax());
    }

    m_hasModel = true;
    update();
}
