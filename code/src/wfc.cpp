#include "wfc.h"
#include <random>
#include <algorithm>

Wfc::Wfc(Grid& grid) : m_grid(grid) {}



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

void print(const QSet<int>& set) {
    QTextStream out(stdout);
    for (auto it = set.constBegin(); it != set.constEnd(); ++it) {
        out << *it << " ";
    }
    out << endl;
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
                        for(int j=0;j<voisins.size();j++){
                            m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).entropy++;
                        }
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
                                for(int j=0;j<voisins.size();j++){
                                    m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).entropy++;
                                }
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
    m_grid.printGrid();
}

void Wfc::runWFC(Grid& grid,int k,QVector<TileModel> &modeles){



    initWFC(k,modeles);
    bool isFull=false;
    std::random_device rd;
    std::mt19937 gen(rd());
    QSet<QVector3D> cellsDone;
    std::cout<<"Debut de run"<<std::endl;
    //Comptage de cellule avec modele :
    int c=0;
    std::cout<<"Dim grid : " << m_grid.getX() <<" "<<m_grid.getY()<<" "<<m_grid.getZ()<<" Total :  "<<m_grid.getX()*m_grid.getY()*m_grid.getZ()<<std::endl;
    for(int x = 0;x<m_grid.getX();x++){
        for(int y = 0;y<m_grid.getY();y++){
            for(int z = 0;z<m_grid.getZ();z++){
                if(m_grid.getCell(x,y,z).hasMesh){
                    c++;
                }
            }
        }
    }
    std::cout<<"comptage initial fait : "<<c<<std::endl;
    //Remplissage grille
    int iterationLimit=m_grid.getX()*m_grid.getY()*m_grid.getZ()*3;
    int iterationCount=0;
    bool isEnd=false;
    while(!isFull&&!isEnd){
        iterationCount++;
        if(iterationCount>iterationLimit){
            isEnd=true;
        }
        //Parcours grille pour meilleure entropie
        int maxEntropy=-1;
        QVector<QVector3D> bestEntropy;
        for(int x = 0;x<m_grid.getX();x++){
            for(int y = 0;y<m_grid.getY();y++){
                for(int z = 0;z<m_grid.getZ();z++){
                    if(m_grid.getCell(x,y,z).entropy>maxEntropy&&!m_grid.getCell(x,y,z).hasMesh){
                        maxEntropy=m_grid.getCell(x,y,z).entropy;
                        std::cout<<x<<" "<<y<<" "<<z<<std::endl;
                    }
                }
            }
        }
        std::cout<<"entropie max : "<<maxEntropy<<std::endl;
        //On chosiit les sommets qu'on garde, donc ceux avec entropie max
        for(int x = 0;x<m_grid.getX();x++){
            for(int y = 0;y<m_grid.getY();y++){
                for(int z = 0;z<m_grid.getZ();z++){
                    if(m_grid.getCell(x,y,z).entropy==maxEntropy&&!m_grid.getCell(x,y,z).hasMesh){
                        bestEntropy.push_back(QVector3D(x,y,z));
                        std::cout<<x<<" "<<y<<" "<<z<<std::endl;
                    }
                }
            }
        }
        bool isSet=false;
        //boucle du choix de sommet avec bonne entropie que l'on teste
        std::cout<<"while sur la position"<<std::endl;

        while(!isSet&&!bestEntropy.isEmpty()){
            std::uniform_int_distribution<> disEntropy(0,bestEntropy.size()-1);
            int randomPossibility=disEntropy(gen);
            std::cout<<"indice dans bestEntropy : " << randomPossibility<<std::endl;
            //On a la cellule dans laquelle on va essayer de mettre un modele
            int randomX=bestEntropy[randomPossibility].x();
            int randomY=bestEntropy[randomPossibility].y();
            int randomZ=bestEntropy[randomPossibility].z();
            std::cout<<randomX<<" "<<randomY<<" "<<randomZ<<std::endl;
            //On retire le candidat des candidats possibles
            bestEntropy.removeAt(randomPossibility);
            //Voisins de cellule choisie par entropie
            QVector<QVector3D> voisins;
            voisins.push_back(QVector3D(std::min(randomX+1,m_grid.getX()-1),randomY,randomZ));
            voisins.push_back(QVector3D(std::max(randomX-1,0),randomY,randomZ));
            voisins.push_back(QVector3D(randomX,std::min(randomY+1,m_grid.getY()-1),randomZ));
            voisins.push_back(QVector3D(randomX,std::max(randomY-1,0),randomZ));
            voisins.push_back(QVector3D(randomX,randomY,std::min(randomZ+1,m_grid.getZ()-1)));
            voisins.push_back(QVector3D(randomX,randomY,std::max(randomZ-1,0)));
            removeElementFromVector(voisins,QVector3D(randomX,randomY,randomZ));
            //recupération des règles des voisins

            //Vérification de règle dans run ne fonctionne pas
            std::cout<<"regles"<<std::endl;
            QVector<QSet<int>> ruleSets;
            for(int j=0;j<voisins.size();j++){
                if(m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).hasMesh){
                    ruleSets.push_back(m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).object.tileModel()->getRules());
                    print(m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).object.tileModel()->getRules());
                }
            }
            //Liste des modeles possibles sur la cellule selon les règles des voisins présents
            QSet<int> possibleModeles;
            if (!ruleSets.isEmpty()) {
                possibleModeles = ruleSets[0];
                for (int i = 1; i < ruleSets.size(); ++i) {
                    possibleModeles = possibleModeles.intersect(ruleSets[i]);
                }
            }
            std::cout<<"Modeles possibles : "<<std::endl;
            print(possibleModeles);
            if(possibleModeles.isEmpty()){
                isSet=false;
                std::cout<<"Aucun modele possible"<<std::endl;
            }
            else{
                QList<int> list = possibleModeles.toList();
                std::cout<<"Cellule remplie : "<<randomX<<" "<<randomY<<" "<<randomZ<<std::endl;
                std::cout<<possibleModeles.size()<<std::endl;
                std::uniform_int_distribution<> disModele(0,list.size()-1);
                int randomModel=disModele(gen);
                TileInstance instance(&modeles[list[randomModel]], Transform());
                std::cout<<"Modele choisi : " << randomModel<<" "<<modeles[list[randomModel]].getId()<<std::endl;
                m_grid.setObject(instance,randomX,randomY,randomZ);
                isSet=true;
                for(int j=0;j<voisins.size();j++){
                    m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).entropy++;
                }
                c++;
            }
            std::cout<<"Fin de boucle while sur position"<<std::endl;
        }
        //Check de remplissage de grille
        if(c>=m_grid.getX()*m_grid.getY()*m_grid.getZ()){
            std::cout<<"FIn de programme normale"<<std::endl;
            isFull=true;
        }
        m_grid.printGrid();
        std::cout<<isFull<<std::endl;
    }
}



