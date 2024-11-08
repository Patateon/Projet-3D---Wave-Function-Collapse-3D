#include "transform.h"

Transform::Transform()
{
    m_position = QVector3D(0.0f, 0.0f, 0.0f);
    m_scale = QVector3D(1.0f, 1.0f, 1.0f);
    m_rotation = QQuaternion();
}

QMatrix4x4 Transform::getLocalModel(){
    /*
     * TODO
    */
    return QMatrix4x4();
}
