#include "tileinstance.h"

TileInstance::TileInstance(TileModel* tileModel)
{
    m_tileModel = tileModel;
    test=11;
}

TileInstance::TileInstance(
        TileModel* tileModel, Transform transform)

{
    test=11;
    m_tileModel = tileModel;
    m_transform = transform;
}
