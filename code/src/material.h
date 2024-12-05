#ifndef MATERIAL_H
#define MATERIAL_H

#include <QVector3D>


class Material
{
public:
    Material();
    ~Material();

private:
    QVector3D m_ambient_color;
    QVector3D m_diffuse_color;
    QVector3D m_specular_color;
};

#endif // MATERIAL_H
