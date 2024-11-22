#include "wfc.h"
#include <random>

wfc::wfc(Grid& grid) {
    m_grid=grid;
}

wfc::runWFC(Grid& grid,int k,QVector<TileModel> &modeles){
    initWFC(k,modeles);

}

wfc::initWFC(int k,QVector<TileModel> &modeles){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disX(0,m_grid.getX());
    std::uniform_int_distribution<> disY(0,m_grid.getY());
    std::uniform_int_distribution<> disZ(0,m_grid.getZ());
    std::uniform_int_distribution<> disModele(0,modeles.size()-1);
    int randomX,randomY,randomZ,randomModel;
    bool objectSet=false;
    for(int i = 0 ; i<k;i++){
        while(!objectSet){
            objectSet=true;
            randomX=disX(gen);
            randomY=disY(gen);
            randomZ=disZ(gen);
            if(!m_grid.getCell(randomX,randomY,randomZ).hasMesh()){
                randomModel=disModele(gen);
                //Check voisins pour les règles 4 connexité dans grille si possible
                TileInstance instance(&modeles[randomModel], Transform());
                m_grid.setObject(instance,randomX,randomY,randomZ);
            }
            else{
                objectSet=false;
            }
        }
    }
}
