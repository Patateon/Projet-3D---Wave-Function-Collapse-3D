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
#include <QSplitter>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>

#include "qt/QSmartAction.h"

#include "basicviewer.h"
#include "grid.h"
#include "wfc.h"


class MainViewer : public QGLViewer , public QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

    Mesh mesh;
    Grid * grid;
    Wfc * wfc;

    QOpenGLShaderProgram *program = nullptr;
    QOpenGLShaderProgram *gridLineShader = nullptr;

    QWidget * controls;

private :
    float m_scale_normal = 1.0;
    bool m_display_normal = true;
    bool m_wired = false;
    bool m_showgrid = true;
    bool m_model_layout_initialized = false;

    QMainWindow *m_mainWindow;
    QVBoxLayout *m_modelsLayout;

    QVector<TileModel> m_modeles;
    QListWidget *modelList;

public :

    explicit MainViewer(QGLWidget *parent = nullptr);
    ~MainViewer();

    void add_actions_to_toolBar(QToolBar *toolBar);
    void pickBackgroundColor();
    void adjustCamera( point3d const & bb , point3d const & BB );

    void initModelsViewer();
    void initializeGrid();
    void initializeRandomGrid(uint dimension, float spacing);
    void initializeBasicWFC(uint dimension, float spacing);
    void initGrid(uint dimension,float spacing);
    void initializeProgramShader();
    void initializeGridLineShader();
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

    QVector<TileModel> getModeles();
    void setModeles(QVector<TileModel> modeles);

    void setMainWindow(QMainWindow *mainWindow);

signals:
    void windowTitleUpdated( const QString & );

public slots:
    void open_mesh();
    void create_initialization_grid();

    void save_mesh();

    void showControls();

    void onModelDoubleClicked(QListWidgetItem *item);
    void onOrientationButtonClicked(int modelIndex, const QString &axis, int angle);
    void updateButtonColor(QPushButton *button, const QString &axis, int angle, const QVector<bool> &rotx, const QVector<bool> &roty, const QVector<bool> &rotz);
    void updateButtonColors(const QVector<bool> &rotx, const QVector<bool> &roty, const QVector<bool> &rotz);
};

#endif // MAINVIEWER_H
