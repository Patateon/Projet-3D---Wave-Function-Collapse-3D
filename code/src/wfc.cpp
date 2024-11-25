#include "wfc.h"
#include <random>
#include <algorithm>

Wfc::Wfc(Grid& grid) : m_grid(grid) {}

void Wfc::runWFC(Grid& grid,int k,QVector<TileModel> &modeles){
    initWFC(k,modeles);

}

//Fonction pour utiliser un QVector3D comme clé dans un QSet
inline uint qHash(const QVector3D &vector, uint seed = 0) {
    return qHash(QPair<int, int>(qHash(vector.x(), seed), qHash(vector.y(), seed)), seed) ^ qHash(vector.z(), seed);
}

void removeElementFromVector(QVector<QVector3D>& vec, const QVector3D& toRemove) {
    for(int i=0;i<3;i++){
        int index = vec.indexOf(toRemove);
        if (index != -1)
            vec.removeAt(index);
    }
}

void Wfc::initWFC(int k,QVector<TileModel> &modeles){
    if(k>=(m_grid.getX()*m_grid.getY()*m_grid.getZ())){
        std::cout<<"Taille de la grille : "<<m_grid.getX()*m_grid.getY()*m_grid.getZ()<<" k trop grand"<<std::endl;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disX(0,m_grid.getX()-1);
    std::uniform_int_distribution<> disY(0,m_grid.getY()-1);
    std::uniform_int_distribution<> disZ(0,m_grid.getZ()-1);
    std::uniform_int_distribution<> disModele(0,modeles.size()-1);
    int randomX,randomY,randomZ,randomModel;
    QSet<int> rules;
    bool objectSet=false;
    QVector<QVector3D> voisins;
    int id;
    bool ruleBroken = false;
    QSet<QVector3D> cellsDone;

    for(int i = 0 ; i<k;i++){
        objectSet=false;
        while(!objectSet){
            objectSet=true;
            randomX=disX(gen);
            randomY=disY(gen);
            randomZ=disZ(gen);
            std::cout << "Random Position: (" << randomX << ", " << randomY << ", " << randomZ << ")" << std::endl;
            //Check si cellule choisie random contient deja un objet ou si on l'a déja testée
            if(!m_grid.getCell(randomX,randomY,randomZ).hasMesh&&!cellsDone.contains(QVector3D(randomX,randomY,randomZ))){
                bool endModelChoice=false;
                int iterationModelCount=0;
                QVector<QSet<int>> ruleSets;
                //Check voisins pour les règles 6 connexité dans grille si possible
                voisins.push_back(QVector3D(std::min(randomX+1,m_grid.getX()-1),randomY,randomZ));
                voisins.push_back(QVector3D(std::max(randomX-1,0),randomY,randomZ));
                voisins.push_back(QVector3D(randomX,std::min(randomY+1,m_grid.getY()-1),randomZ));
                voisins.push_back(QVector3D(randomX,std::max(randomY-1,0),randomZ));
                voisins.push_back(QVector3D(randomX,randomY,std::min(randomZ+1,m_grid.getZ()-1)));
                voisins.push_back(QVector3D(randomX,randomY,std::max(randomZ-1,0)));
                //recupération des règles des voisins
                removeElementFromVector(voisins,QVector3D(randomX,randomY,randomZ));

                for(int j=0;j<voisins.size();j++){
                    if(m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).hasMesh){
                        ruleSets.push_back(m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).object.tileModel()->getRules());
                    }
                }
                while(!endModelChoice){
                    //Seuil d'arrêt si on est dans une situation qui ne résolve pas, on change de cellule
                    iterationModelCount++;
                    if(iterationModelCount>modeles.size()){
                        cellsDone.insert(QVector3D(randomX,randomY,randomZ));
                        break;
                    }
                    randomModel=disModele(gen);
                    rules=modeles[randomModel].getRules();
                    //On check si les voisins peuvent avoir le modele que l'on a tiré au hasard comme voisin
                    if(ruleSets.isEmpty()){
                        objectSet=true;
                        endModelChoice=true;
                        TileInstance instance(&modeles[randomModel], Transform());
                        m_grid.setObject(instance,randomX,randomY,randomZ);
                        m_grid.getCell(randomX,randomY,randomZ).hasMesh=true;
                        m_grid.getCell(randomX,randomY,randomZ).object.test=randomModel;
                        std::cout<<"Une instance est mise car pas de regle autour"<<std::endl;

                    }
                    else{
                        if(ruleCheck(ruleSets,randomModel)){
                            //On check si le modele que l'on a choisi peut avoir l'id des voisins en voisin
                            for(int j=0;j<voisins.size();j++){
                                if(m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).hasMesh){
                                    id=m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).object.tileModel()->id();
                                    if(!rules.contains(id)){
                                        ruleBroken=true;
                                        break;
                                    }
                                }
                            }
                            //On a trouvé un modele qui n'enfreint pas de regle, on sort des boucles while et on set le modele
                            if(!ruleBroken){
                                objectSet=true;
                                endModelChoice=true;
                                TileInstance instance(&modeles[randomModel], Transform());
                                m_grid.setObject(instance,randomX,randomY,randomZ);
                                m_grid.getCell(randomX,randomY,randomZ).hasMesh=true;
                                m_grid.getCell(randomX,randomY,randomZ).object.test=randomModel;
                                std::cout<<"Une instance est mise apres check regle"<<std::endl;
                            }
                        }
                    }

                }
                voisins.clear();
            }
            else{
                cellsDone.insert(QVector3D(randomX,randomY,randomZ));
                objectSet=false;
            }
        }
    }
}

bool Wfc::ruleCheck(QVector<QSet<int>> voisins,int id_center){
    bool ruleBroken=false;
    for(int i=0;i<voisins.size();i++){
        if(!voisins[i].contains(id_center)){
            ruleBroken=true;
            break;
        }
    }
    return !ruleBroken;
}

