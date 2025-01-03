#ifndef BASICVIEWER_H
#define BASICVIEWER_H

#include <QOpenGLFunctions_4_3_Core>
#include <QGLViewer/qglviewer.h>

#include <tilemodel.h>
#include <QOpenGLShaderProgram>

class BasicViewer : public QGLViewer , public QOpenGLFunctions_4_3_Core
{
public:
    TileModel & tileModel() {return *m_tilemodel;}
    void setTileModel(TileModel * tileModel);

    virtual void init();
    virtual void draw();

    void initializeShader();
    void adjustCamera(QVector3D const &bbmin, QVector3D const &bbmax);

private:
    QOpenGLShaderProgram *m_program = nullptr;
    TileModel *m_tilemodel;

    bool m_hasModel = false;
    bool m_isInitialized = false;
};

#endif // BASICVIEWER_H
