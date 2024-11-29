#ifndef MAINVIEWER_H
#define MAINVIEWER_H


// Mesh stuff:
#include "Mesh.h"

// Parsing:
#include "BasicIO.h"

// opengl and basic gl utilities:
#define GL_GLEXT_PROTOTYPES
#include <gl/openglincludeQtComp.h>
#include <GL/glext.h>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLFunctions>
#include <QGLViewer/qglviewer.h>

#include <gl/GLUtilityMethods.h>

// Qt stuff:
#include <QFormLayout>
#include <QToolBar>
#include <QColorDialog>
#include <QFileDialog>
#include <QKeyEvent>
#include <QInputDialog>
#include <QLineEdit>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

#include "qt/QSmartAction.h"
#include "grid.h"
#include "wfc.h"


class MainViewer : public QGLViewer , public QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

    Mesh mesh;
    Grid * grid;
    Wfc * wfc;

    QOpenGLShaderProgram *program = nullptr;

    GLuint vertexPosition = 0;
    GLuint vertexNormal = 0;
    GLuint vertexColor = 0;

    QWidget * controls;

public :

    MainViewer(QGLWidget * parent = NULL) : QGLViewer(parent) , QOpenGLFunctions_4_3_Core() {
    }
    ~MainViewer();

    void add_actions_to_toolBar(QToolBar *toolBar);
    void pickBackgroundColor();
    void adjustCamera( point3d const & bb , point3d const & BB );

    void initializeModels();
    void initializeGrid();
    void initializeRandomGrid(uint dimension, float spacing);
    void initializeBasicWFC(uint dimension, float spacing);

    void initializeProgramShader();
    void initializeShaders();

    void init() override;
    void draw() override;

    QString helpString() const override;

    void updateTitle( QString text );

    void keyPressEvent( QKeyEvent * event ) override;
    void mouseDoubleClickEvent( QMouseEvent * e ) override;
    void mousePressEvent(QMouseEvent* e ) override;
    void mouseMoveEvent(QMouseEvent* e  ) override;
    void mouseReleaseEvent(QMouseEvent* e  ) override;

signals:
    void windowTitleUpdated( const QString & );

public slots:
    void open_mesh();
    void save_mesh();

    void showControls();

    void saveCameraInFile(const QString &filename);
    void openCameraFromFile(const QString &filename);

    void openCamera();
    void saveCamera();

    void saveSnapShotPlusPlus();
};

#endif // MAINVIEWER_H
