#include "transform.h"

Transform::Transform()
{
    m_translation = QVector3D(0.0f, 0.0f, 0.0f);
    m_scale = QVector3D(1.0f, 1.0f, 1.0f);
    m_rotation = QQuaternion();
}

QMatrix4x4 Transform::getLocalModel(){
    // Compute local model matrix by applying
    // Scale then Rotation then Translation

    m_model.scale(m_scale);
    m_model.translate(m_translation);
    m_model.rotate(m_rotation);

    return m_model;
}



