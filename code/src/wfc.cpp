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
//        std::cout<<"Taille de la grille : "<<m_grid.getX()*m_grid.getY()*m_grid.getZ()<<" k trop grand"<<std::endl;
    }
    //Pondération de la selection aléatoire de modele par la taille de leur rules, i.e, le nombre de voisins possibles
    //Augmente les chances de configuration initiale viable

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disX(0,m_grid.getX()-1);
    std::uniform_int_distribution<> disY(0,m_grid.getY()-1);
    std::uniform_int_distribution<> disZ(0,m_grid.getZ()-1);

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
//            std::cout << "Random Position: (" << randomX << ", " << randomY << ", " << randomZ << ")" << std::endl;
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

                //Check voisin cube 3*3*3 pour voir si il y a deja un modele restreint dans la zone, si oui on ne prend pas les modeles restreint dans possiblemodele
                bool hasType1=m_grid.isTypeClose(randomX,randomY,randomZ,1);
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
                    //Récupération des modeles avec type different de 1 pour prendre seulement parmi eux
                    QVector<TileModel> modeleValid;
                    if(hasType1){
                        for(int j = 0 ;j<modeles.size();j++){
                            if(modeles[j].getType()==1){
                                modeleValid.push_back(modeles[j]);
                            }
                        }
                    }
                    else{
                        modeleValid=modeles;
                    }
                    int total = 0;
                    for ( auto& modele : modeleValid) {
                        total += modele.getRules().size();
                    }

                    std::vector<float> weights;
                    for ( auto& modele : modeleValid) {
                        weights.push_back(static_cast<float>(modele.getRules().size()) / total);
                    }
                    std::discrete_distribution<> disModele(weights.begin(), weights.end());
                    randomModel=disModele(gen);
                    rules=modeles[randomModel].getRules();
                    //On check si les voisins peuvent avoir le modele que l'on a tiré au hasard comme voisin
                    if(ruleSets.isEmpty()){
                        objectSet=true;
                        endModelChoice=true;
                        TileModel model = modeles[randomModel];
                        //Récupérations rotations possibles
                        QVector<bool> x_rot =model.getXRot();//indice 0 = 0 degré , 1 90 , 2 180 , 3 270, fin
                        QVector<bool> y_rot =model.getYRot();
                        QVector<bool> z_rot =model.getZRot();
                        QVector<int> x_possible_rot;
                        QVector<int> y_possible_rot;
                        QVector<int> z_possible_rot;
                        //rotations possibles véritables
                        for(int j=0;j<4;j++){
                            if(x_rot[j]==1){
                                x_possible_rot.push_back(j*90);
                            }
                            if(y_rot[j]==1){
                                y_possible_rot.push_back(j*90);
                            }
                            if(z_rot[j]==1){
                                z_possible_rot.push_back(j*90);
                            }
                        }
                        //Choix au hasard d'une rotation pour le modèle;
                        std::uniform_int_distribution<> disRotX(0,x_possible_rot.size()-1);
                        std::uniform_int_distribution<> disRotY(0,y_possible_rot.size()-1);
                        std::uniform_int_distribution<> disRotZ(0,z_possible_rot.size()-1);
                        float x_rot_value=x_possible_rot[disRotX(gen)];
                        float y_rot_value=x_possible_rot[disRotY(gen)];
                        float z_rot_value=x_possible_rot[disRotZ(gen)];
                        Transform transform;
                        TileInstance instance(&model, transform);
                        m_grid.setObject(instance,randomX,randomY,randomZ,x_rot_value,y_rot_value,z_rot_value);
                        for(int j=0;j<voisins.size();j++){
                            m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).entropy++;
                        }
//                        std::cout<<"Une instance est mise car pas de regle autour"<<std::endl;

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
                                TileInstance instance(&modeles[randomModel], Transform());//Ajout rotation au hasad parmi m_x m_y m_z
                                m_grid.setObject(instance,randomX,randomY,randomZ,0,0,0);
                                for(int j=0;j<voisins.size();j++){
                                    m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).entropy++;
                                }
//                                std::cout<<"Une instance est mise apres check regle"<<std::endl;
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
//    m_grid.printGrid();
}

void Wfc::runWFC(int k,QVector<TileModel> &modeles){

    for(int i=0;i<30;i++){
        initWFC(k,modeles);
        bool isFull=false;
        std::random_device rd;
        std::mt19937 gen(rd());
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
        //std::cout<<"comptage initial fait : "<<c<<std::endl;
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
                            //std::cout<<x<<" "<<y<<" "<<z<<std::endl;
                        }
                    }
                }
            }
            //std::cout<<"entropie max : "<<maxEntropy<<std::endl;
            //On chosiit les sommets qu'on garde, donc ceux avec entropie max
            for(int x = 0;x<m_grid.getX();x++){
                for(int y = 0;y<m_grid.getY();y++){
                    for(int z = 0;z<m_grid.getZ();z++){
                        if(m_grid.getCell(x,y,z).entropy==maxEntropy&&!m_grid.getCell(x,y,z).hasMesh){
                            bestEntropy.push_back(QVector3D(x,y,z));
                            //std::cout<<x<<" "<<y<<" "<<z<<std::endl;
                        }
                    }
                }
            }
            bool isSet=false;
            //boucle du choix de sommet avec bonne entropie que l'on teste
            //std::cout<<"while sur la position"<<std::endl;

            while(!isSet&&!bestEntropy.isEmpty()){
                std::uniform_int_distribution<> disEntropy(0,bestEntropy.size()-1);
                int randomPossibility=disEntropy(gen);
                //std::cout<<"indice dans bestEntropy : " << randomPossibility<<std::endl;
                //On a la cellule dans laquelle on va essayer de mettre un modele
                int randomX=bestEntropy[randomPossibility].x();
                int randomY=bestEntropy[randomPossibility].y();
                int randomZ=bestEntropy[randomPossibility].z();
                //std::cout<<randomX<<" "<<randomY<<" "<<randomZ<<std::endl;
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

                //Check voisin cube 3*3*3 pour voir si il y a deja un modele restreint dans la zone, si oui on ne prend pas les modeles restreint dans possiblemodele
                bool hasType1=m_grid.isTypeClose(randomX,randomY,randomZ,1);
                //std::cout<<"regles"<<std::endl;
                QVector<QSet<int>> ruleSets;
                for(int j=0;j<voisins.size();j++){
                    if(m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).hasMesh){
                        ruleSets.push_back(m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).object.tileModel()->getRules());
                        //print(m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).object.tileModel()->getRules());
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
                //Si il y a un type 1 dans le cube 3*3*3 on retire les type 1 des candidats potentiels
                if(hasType1){
                    foreach (int value, possibleModeles) {
                        if(modeles[value].getType()==1){
                            possibleModeles.remove(value);
                        }
                    }
                }
                //std::cout<<"Modeles possibles : "<<std::endl;
                //print(possibleModeles);
                if(possibleModeles.isEmpty()){
                    isSet=false;
                    //std::cout<<"Aucun modele possible"<<std::endl;

                }
                else{
                    QList<int> list = possibleModeles.toList();
                    //std::cout<<"Cellule remplie : "<<randomX<<" "<<randomY<<" "<<randomZ<<std::endl;
                    //std::cout<<possibleModeles.size()<<std::endl;
                    std::uniform_int_distribution<> disModele(0,list.size()-1);
                    int randomModel=disModele(gen);
                    Transform transform;//si type 1 dans 6 voisinage copier la rotation de celui ci et la mettre sur ca
                    TileInstance instance(&modeles[list[randomModel]], transform);//Ajout rotation au hasad parmi m_x m_y m_z niveau transform
                    //std::cout<<"Modele choisi : " << randomModel<<" "<<modeles[list[randomModel]].id()<<std::endl;
                    float x_deg,y_deg,z_deg=0;

                    for(int j=0;j<voisins.size();j++){
                        // std::cout<<"zone de bug"<<std::endl;
                        if(m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).hasMesh&&m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).object.tileModel()->getType()==1){
                            m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).object.transform().rotation().getEulerAngles(&x_deg,&y_deg,&z_deg);
                        }
                        // std::cout<<"fin zone de bug"<<std::endl;
                    }
                    m_grid.setObject(instance,randomX,randomY,randomZ,0,0,0);
                    isSet=true;
                    for(int j=0;j<voisins.size();j++){
                        m_grid.getCell(voisins[j].x(),voisins[j].y(),voisins[j].z()).entropy++;
                    }
                    c++;
                }
                //std::cout<<"Fin de boucle while sur position"<<std::endl;
            }
            //Check de remplissage de grille
            if(c>=m_grid.getX()*m_grid.getY()*m_grid.getZ()){
                std::cout<<"FIn de programme normale"<<std::endl;
                isFull=true;
            }
            //m_grid.printGrid();
            // std::cout<<isFull<<std::endl;
        }
        if(isFull){
            std::cout<<"Fin apres "<<i<<" iterations"<<std::endl;
            break;
        }
        else{
            //Ici supprimer les éléments de la grille
            m_grid.clean();
        }

    }

}



