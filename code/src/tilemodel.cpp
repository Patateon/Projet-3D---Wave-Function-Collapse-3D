#include "tilemodel.h"

#include "BasicIO.h"

TileModel::TileModel(uint id)
{
    m_id = id;
}

TileModel::TileModel(uint id, QString filename)
{
    m_id = id;
    setMesh(filename);
}

TileModel::TileModel(uint id, QSet<int> rules)
{
    m_id = id;
    m_rules=rules;
}

TileModel::~TileModel(){

}

void TileModel::setMesh(QString filename)
{
    if (filename.endsWith(".off")){
        OFFIO::openTriMesh(filename.toStdString(),
                                     m_mesh.vertices,
                                     m_mesh.triangles);
    }else if (filename.endsWith(".obj")){
        OBJIO::openTriMesh(filename.toStdString(),
                                     m_mesh.vertices,
                                     m_mesh.triangles);
    }

    computeBoundingBox();
}

uint TileModel::getId()const{
    return m_id;
}

bool TileModel::operator<(const TileModel & other) const{
    return m_id < other.m_id;
}

void TileModel::computeBoundingBox() {

    if (mesh().vertices.size() <= 0){
        m_bbmin = QVector3D(0.0f, 0.0f, 0.0f);
        m_bbmax = QVector3D(0.0f, 0.0f, 0.0f);
        return;
    }

    m_bbmin = QVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
    m_bbmax = QVector3D(FLT_MIN, FLT_MIN, FLT_MIN);

    for(uint i = 0; i < mesh().vertices.size(); i++){
        for(uint k = 0; k < 3; k++){
            if (mesh().vertices[i][k] > m_bbmax[k]){
                m_bbmax[k] = mesh().vertices[i][k];
            }
            if (mesh().vertices[i][k] < m_bbmin[k]){
                m_bbmin[k] = mesh().vertices[i][k];
            }
        }
    }
}


