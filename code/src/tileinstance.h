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
    const TileModel* tileModel() const { return m_tileModel; }
    Transform & transform() {return m_transform;}
    int test;

    bool isValid() const {
        return m_tileModel != nullptr;
    }



private:
    // Reference to model
    TileModel* m_tileModel = nullptr;


    // Instance informations
    Transform m_transform;
};

#endif // TILEINSTANCE_H
