#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>

class Transform
{
public:
    Transform();

    QVector3D & position() {return m_position;}
    QQuaternion & rotation() {return m_rotation;}
    QVector3D & scale() {return m_scale;}

    QMatrix4x4 getLocalModel();

private:
    QVector3D m_position;
    QQuaternion m_rotation;
    QVector3D m_scale;

    QMatrix4x4 m_model;
};

#endif // TRANSFORM_H
