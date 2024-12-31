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
    TileModel(uint id, QVector<QSet<int>> rules);
    ~TileModel();

    // Getter/Setter
    const uint & id() {return m_id;}
    uint id() const {return m_id;}
    Mesh & mesh() {return m_mesh;}
    const QVector3D & bbmin() {return m_bbmin;}
    const QVector3D & bbmax() {return m_bbmax;}

    // Mesh file handling
    void setMesh(Mesh *mesh) {m_mesh = *mesh;}
    void setMesh(QString filename);
    void loadOBJ(QString filename);

    void computeBoundingBox();
    QVector<QSet<int>*> getRules();
    QSet<int> getXMinus();
    QSet<int> getXPlus();
    QSet<int> getYMinus();
    QSet<int> getYPlus();
    QSet<int> getZMinus();
    QSet<int> getZPlus();
    void setRules(QSet<int> rules_xminus,QSet<int> rules_xplus,QSet<int> rules_yminus,QSet<int> rules_yplus,QSet<int> rules_zminus,QSet<int> rules_zplus);
    QVector<bool> getXRot();
    QVector<bool> getYRot();
    QVector<bool> getZRot();
    void setRots(QVector<bool> rotx,QVector<bool> roty,QVector<bool> rotz);
    int getType();
    void setType(QVector<TileModel> &modeles,int mode);
    bool operator<(const TileModel & other) const;
private:
    uint m_id;
    Mesh m_mesh;
    QSet<int> m_rules_xminus;//Contient les indices de modele autorisé en x négatif de ce modele
    QSet<int> m_rules_xplus;
    QSet<int> m_rules_yminus;
    QSet<int> m_rules_yplus;
    QSet<int> m_rules_zminus;
    QSet<int> m_rules_zplus;
    QVector<bool> m_rotx;//regles sur rotation autorisée , 90 180 270
    QVector<bool> m_roty;
    QVector<bool> m_rotz;
    uint m_type;
    QVector3D m_bbmin;
    QVector3D m_bbmax;
};

#endif // TILEMODEL_H
