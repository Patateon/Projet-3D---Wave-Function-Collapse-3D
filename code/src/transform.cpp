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

    m_model.translate(m_translation);
    m_model.rotate(m_rotation);
    m_model.scale(m_scale);

    return m_model;
}

QVector3D Transform::getRotationAngles() const {
    // Convert the quaternion to a 4x4 rotation matrix
    QMatrix4x4 rotationMatrix;
    rotationMatrix.rotate(m_rotation);

    // Extract the rotation angles in radians
    float sy = sqrt(rotationMatrix(0, 0) * rotationMatrix(0, 0) + rotationMatrix(1, 0) * rotationMatrix(1, 0));

    bool singular = sy < 1e-6; // If sy is close to zero, we consider the matrix to be singular

    float x, y, z;
    if (!singular) {
        x = atan2(rotationMatrix(2, 1), rotationMatrix(2, 2));
        y = atan2(-rotationMatrix(2, 0), sy);
        z = atan2(rotationMatrix(1, 0), rotationMatrix(0, 0));
    } else {
        x = atan2(-rotationMatrix(1, 2), rotationMatrix(1, 1));
        y = atan2(-rotationMatrix(2, 0), sy);
        z = 0;
    }

    // Convert the angles from radians to degrees
    x = qRadiansToDegrees(x);
    y = qRadiansToDegrees(y);
    z = qRadiansToDegrees(z);

    return QVector3D(x, y, z);
}

