#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>
#include <QtMath>
class Transform
{
public:
    Transform();

    QVector3D & translation() {return m_translation;}
    QQuaternion & rotation() {return m_rotation;}
    QVector3D & scale() {return m_scale;}

    QMatrix4x4 getLocalModel();
    QVector3D getRotationAngles() const;


private:
    QVector3D m_translation;
    QQuaternion m_rotation;
    QVector3D m_scale;

    QMatrix4x4 m_model;
};

#endif // TRANSFORM_H
