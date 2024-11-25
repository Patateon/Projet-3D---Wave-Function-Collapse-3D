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
}

uint TileModel::getId()const{
    return m_id;
}

QSet<int> TileModel::getRules(){
    return m_rules;
}

void TileModel::setRules(QSet<int> rules){
    m_rules=rules;
}
