#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Mesh.h"
#include "transform.h"

class GameObject
{
public:
    GameObject();

    void display();

    Mesh & mesh() {return m_mesh;}
    Transform & transform() {return m_transform;}

private:
    Mesh m_mesh;
    Transform m_transform;
};

#endif // GAMEOBJECT_H
