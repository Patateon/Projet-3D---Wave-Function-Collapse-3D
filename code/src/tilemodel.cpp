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

uint TileModel::getId(){
    return m_id;
}

bool TileModel::operator<(const TileModel & other) const{
    return m_id < other.m_id;
}
