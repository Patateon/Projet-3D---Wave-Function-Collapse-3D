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
    ~TileModel();

    // Getter/Setter
    const uint & id() {return m_id;}
    Mesh & mesh() {return m_mesh;}
    const QVector3D & bbmin() {return m_bbmin;}
    const QVector3D & bbmax() {return m_bbmax;}

    // Mesh file handling
    void setMesh(Mesh *mesh) {m_mesh = *mesh;}
    void setMesh(QString filename);

    void computeBoundingBox();

    uint getId();

    bool operator<(const TileModel & other) const;
private:
    uint m_id;
    Mesh m_mesh;

    QVector3D m_bbmin;
    QVector3D m_bbmax;
};

#endif // TILEMODEL_H
