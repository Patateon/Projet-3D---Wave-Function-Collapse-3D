#ifndef TILEMODEL_H
#define TILEMODEL_H

#include "Mesh.h"
#include "transform.h"

class TileModel
{
public:
    // Constructor/Desctructor
    TileModel(uint id);
    TileModel(uint id, QString filename);
    TileModel(uint id, QSet<int> rules);
    ~TileModel();

    // Getter/Setter
    const uint & id() {return m_id;}
    Mesh & mesh() {return m_mesh;}

    // Mesh file handling
    void setMesh(Mesh *mesh) {m_mesh = *mesh;}
    void setMesh(QString filename);

    uint getId()const;
    QSet<int> getRules();
    void setRules(QSet<int> rules);
private:
    uint m_id;
    Mesh m_mesh;
    QSet<int> m_rules;//Contient les indices de modele autorisé dans les cellules adjacentes
};

#endif // TILEMODEL_H
