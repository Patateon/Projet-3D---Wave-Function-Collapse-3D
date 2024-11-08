#ifndef TILEINSTANCE_H
#define TILEINSTANCE_H

#include "tilemodel.h"

class TileInstance
{
public:
    // Constructor
    TileInstance(TileModel* tileModel);
    TileInstance(TileModel* tileModel, Transform transform);

    // Getter/Setter
    TileModel *& tileModel() {return m_tileModel;}
    Transform & transform() {return m_transform;}

private:
    // Reference to model
    TileModel* m_tileModel = nullptr;

    // Instance informations
    Transform m_transform;
};

#endif // TILEINSTANCE_H
