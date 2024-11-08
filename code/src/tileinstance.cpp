#include "tileinstance.h"

TileInstance::TileInstance(TileModel* tileModel)
{
    m_tileModel = tileModel;
}

TileInstance::TileInstance(
        TileModel* tileModel, Transform transform)
{
    m_tileModel = tileModel;
    m_transform = transform;
}
