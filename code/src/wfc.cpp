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

QSet<int> Wfc::getCorrespondingRules(QVector3D pos,QVector3D posV,TileModel *model){
    QSet<int> set;
    if(pos.x()<posV.x()) set=model->getXMinus();
    if(pos.x()>posV.x()) set= model->getXPlus();
    if(pos.y()<posV.y()) set=model->getYMinus();
    if(pos.y()>posV.y()) set= model->getYPlus();
    if(pos.z()<posV.z()) set= model->getZMinus();
    if(pos.z()>posV.z()) set= model->getZPlus();
    return set;
}

bool contains(const QVector<QVector3D>& vec, const QVector3D& value) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}
int Wfc::findVectorPrio(QVector<QVector3D> vector,QVector<QVector3D> cellsDone){
    QVector<int> indexOf0,indexOf1;
    for(int i=0;i<vector.size();i++){
        if(m_grid.getCell(vector[i].x(),vector[i].y(),vector[i].z()).hasMesh){
            for(int j=0;j<cellsDone.size();j++){
                if(vector[i]==cellsDone[j]){
                    if(m_grid.getCell(vector[i].x(),vector[i].y(),vector[i].z()).object.tileModel()->getType()==0){
                        indexOf0.push_back(j);
                    }
                    else{
                        indexOf1.push_back(j);
                    }
                }
            }
        }
    }
    if(indexOf0.isEmpty()&&indexOf1.isEmpty()){
        qDebug() << "Indexof0 is empty";
        return -1;
    }
    else if(indexOf1.isEmpty()){
        //cherche donc une cellule type0 la plus vieille, donc l'indexe le plus bas
        int min=indexOf0[0];
        for(int i =1;i<indexOf0.size();i++){
            if(indexOf0[i]<min) min=indexOf0[i];
        }
        return min;
    }
    else{
        int min=indexOf1[0];
        for(int i =1;i<indexOf1.size();i++){
            if(indexOf1[i]<min) min=indexOf1[i];
        }
        return min;
    }
}

QVector3D approxAngle(QVector3D &angle) {
    float epsilon = 5.0f;
    float tolerance = 0.1f;
    if (fabs(angle.x() + 90.0f) < tolerance) angle.setX(270.0f);
    if (fabs(angle.y() + 90.0f) < tolerance) angle.setY(270.0f);
    if (fabs(angle.z() + 90.0f) < tolerance) angle.setZ(270.0f);
    for (int i = 0; i < 4; i++) {

        if (angle.x() > (i * 90) - epsilon && angle.x() < (i * 90) + epsilon) {
            angle.setX(i * 90);
        }
        if (angle.y() > (i * 90) - epsilon && angle.y() < (i * 90) + epsilon) {
            angle.setY(i * 90);
        }
        if (angle.z() > (i * 90) - epsilon && angle.z() < (i * 90) + epsilon) {
            angle.setZ(i * 90);
        }
    }
    return angle;
}

//Pour ini et run, potentiel de retirer la vérification de regle dans les 2 sens et seulement se contenter du sens regle voisins
//, récupération de l'intersection car avec la lecture de grille les regles seront cohérentes, i.e , si a peut etre voisin de b , b peut etre voisin de a, automatiquement.
//La verification dans les2 sens était utile lorsque l'on settait les regles a la main a hasard
void Wfc::initWFC(int k, QVector<TileModel*> &modeles, int mode) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disX(0, m_grid.getX() - 1);
    std::uniform_int_distribution<> disY(0, m_grid.getY() - 1);
    std::uniform_int_distribution<> disZ(0, m_grid.getZ() - 1);

    int randomX, randomY, randomZ, randomModel;
    QVector<QSet<int>*> rules;
    bool objectSet = false;
    QVector<QVector3D> voisins;
    int id;
    bool ruleBroken = false;
    QVector<QVector3D> cellsDone;
    //Nombre d'itération limite pour avoir une grille pleine
    for (int i = 0; i < k; i++) {
        objectSet = false;
        while (!objectSet) {
            objectSet = true;
            //Choix de la position à remplir
            if (cellsDone.isEmpty()) {
                randomX = disX(gen);
                randomY = disY(gen);
                randomZ = disZ(gen);
            } else {
                QVector3D pos = cellsDone[0];
                while (cellsDone.contains(pos)) {
                    randomX = disX(gen);
                    randomY = disY(gen);
                    randomZ = disZ(gen);
                    pos = QVector3D(randomX, randomY, randomZ);
                }
            }

            if (!m_grid.getCell(randomX, randomY, randomZ).hasMesh &&
                !cellsDone.contains(QVector3D(randomX, randomY, randomZ))) {
                //variable de limite d'itération sur le choix de modele
                bool endModelChoice = false;
                int iterationModelCount = 0;
                QVector<QSet<int>> ruleSets;
                //Récupération voisins
                voisins.push_back(QVector3D(std::min(randomX + 1, m_grid.getX() - 1), randomY, randomZ));
                if (!contains(voisins, QVector3D(std::max(randomX - 1, 0), randomY, randomZ)))
                    voisins.push_back(QVector3D(std::max(randomX - 1, 0), randomY, randomZ));
                if (!contains(voisins, QVector3D(randomX, std::min(randomY + 1, m_grid.getY() - 1), randomZ)))
                    voisins.push_back(QVector3D(randomX, std::min(randomY + 1, m_grid.getY() - 1), randomZ));
                if (!contains(voisins, QVector3D(randomX, std::max(randomY - 1, 0), randomZ)))
                    voisins.push_back(QVector3D(randomX, std::max(randomY - 1, 0), randomZ));
                if (!contains(voisins, QVector3D(randomX, randomY, std::min(randomZ + 1, m_grid.getZ() - 1))))
                    voisins.push_back(QVector3D(randomX, randomY, std::min(randomZ + 1, m_grid.getZ() - 1)));
                if (!contains(voisins, QVector3D(randomX, randomY, std::max(randomZ - 1, 0))))
                    voisins.push_back(QVector3D(randomX, randomY, std::max(randomZ - 1, 0)));

                removeElementFromVector(voisins, QVector3D(randomX, randomY, randomZ));
                //Récupération des règles des voisins
                for (int j = 0; j < voisins.size(); j++) {
                    if (m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).hasMesh) {
                        if (mode == 0) {
                            QSet set = getCorrespondingRules(
                                QVector3D(randomX, randomY, randomZ),
                                voisins[j],
                                m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).object.tileModel());
                            ruleSets.push_back(set);
                        } else {
                            ruleSets.push_back(
                                m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).object.tileModel()->getXMinus());
                        }
                    }
                }

                while (!endModelChoice) {
                    iterationModelCount++;
                    if (iterationModelCount > modeles.size()) {
                        cellsDone.push_back(QVector3D(randomX, randomY, randomZ));
                        break;
                    }

                    QVector<TileModel*> modeleValid = modeles;
                    int total = 0;
                    //Préparation des poids pour distribution aléatoire pondérée
                    for (auto &modele : modeleValid) {
                        for (int j = 0; j < modele->getRules().size(); j++) {
                            total += modele->getRules()[j]->size();
                        }
                    }

                    std::vector<float> weights;
                    for (auto &modele : modeleValid) {
                        int sizeCheck = 0;
                        for (int j = 0; j < modele->getRules().size(); j++) {
                            sizeCheck += modele->getRules()[j]->size();
                        }
                        weights.push_back(static_cast<float>(sizeCheck) / total);
                    }
                    //Choix au hasard selon les poids du modele à tester
                    std::discrete_distribution<> disModele(weights.begin(), weights.end());
                    randomModel = disModele(gen);
                    rules = modeles[randomModel]->getRules();
                    //Cas ou il n'y a pas de règles donc pas de voisins
                    if (ruleSets.isEmpty()) {
                        //Récupération d'une rotation aléatoire selon règles du modele choisi
                        objectSet = true;
                        endModelChoice = true;
                        TileModel model = *modeles[randomModel];

                        QVector<bool> x_rot = model.getXRot();
                        QVector<bool> y_rot = model.getYRot();
                        QVector<bool> z_rot = model.getZRot();

                        QVector<int> x_possible_rot;
                        QVector<int> y_possible_rot;
                        QVector<int> z_possible_rot;

                        for (int j = 0; j < 4; j++) {
                            if (x_rot[j] == 1) x_possible_rot.push_back(j * 90);
                            if (y_rot[j] == 1) y_possible_rot.push_back(j * 90);
                            if (z_rot[j] == 1) z_possible_rot.push_back(j * 90);
                        }

                        std::uniform_int_distribution<> disRotX(0, x_possible_rot.size() - 1);
                        std::uniform_int_distribution<> disRotY(0, y_possible_rot.size() - 1);
                        std::uniform_int_distribution<> disRotZ(0, z_possible_rot.size() - 1);

                        float x_rot_value = x_possible_rot[disRotX(gen)];
                        float y_rot_value = y_possible_rot[disRotY(gen)];
                        float z_rot_value = z_possible_rot[disRotZ(gen)];

                        Transform transform;
                        TileInstance instance(new TileModel(model), transform);
                        //On set l'objet
                        m_grid.setObject(instance, randomX, randomY, randomZ, x_rot_value, y_rot_value, z_rot_value);
                        cellsDone.push_back(QVector3D(randomX, randomY, randomZ));

                        for (int j = 0; j < voisins.size(); j++) {
                            m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).entropy++;
                        }
                    } else {
                        //Vérification des règles dans l'autre sens, i.e , voisinage correct pour le modele choisi à la position choisie
                            ruleBroken = false;
                            for (int j = 0; j < voisins.size(); j++) {
                                if (m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).hasMesh) {
                                    id = m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).object.tileModel()->id();
                                    QSet set = getCorrespondingRules(voisins[j], QVector3D(randomX, randomY, randomZ), modeles[randomModel]);
                                    if (!set.contains(id)) {
                                        ruleBroken = true;
                                        break;
                                    }
                                }
                            }
                            //Si pas de probleme au niveaux des regles on prépare la rotation
                            if (!ruleBroken) {
                                objectSet = true;
                                endModelChoice = true;
                                TileModel model = *modeles[randomModel];
                                Transform transform;
                                TileInstance instance(new TileModel(model), transform);
                                //On recherche le voisin prioritaire pour copier la rotation
                                float x_deg, y_deg, z_deg = 0;
                                int posPrio = findVectorPrio(voisins, cellsDone);
                                Transform transformCopy = m_grid.getCell(
                                                                    cellsDone[posPrio].x(),
                                                                    cellsDone[posPrio].y(),
                                                                    cellsDone[posPrio].z()).object.transform();
                                QVector3D angles = transformCopy.getRotationAngles();
                                angles=approxAngle(angles);
                                int indexX = std::abs(angles[0] / 90.0f);
                                int indexY = std::abs(angles[1] / 90.0f);
                                int indexZ = std::abs(angles[2] / 90.0f);
                                //Si la rotation du voisins prioritaire est possible pour le modele choisi on la copie
                                if (model.getXRot()[indexX] == 1 && model.getYRot()[indexY] == 1 && model.getZRot()[indexZ] == 1) {
                                    m_grid.getCell(
                                              cellsDone[posPrio].x(),
                                              cellsDone[posPrio].y(),
                                              cellsDone[posPrio].z()).object.transform().rotation().getEulerAngles(&x_deg, &y_deg, &z_deg);
                                //Sinon prise au hasard
                                } else {
                                    QVector<bool> x_rot = model.getXRot();
                                    QVector<bool> y_rot = model.getYRot();
                                    QVector<bool> z_rot = model.getZRot();

                                    QVector<int> x_possible_rot;
                                    QVector<int> y_possible_rot;
                                    QVector<int> z_possible_rot;

                                    for (int j = 0; j < 4; j++) {
                                        if (x_rot[j] == 1) x_possible_rot.push_back(j * 90);
                                        if (y_rot[j] == 1) y_possible_rot.push_back(j * 90);
                                        if (z_rot[j] == 1) z_possible_rot.push_back(j * 90);
                                    }
                                    std::uniform_int_distribution<> disRotX(0, x_possible_rot.size() - 1);
                                    std::uniform_int_distribution<> disRotY(0, y_possible_rot.size() - 1);
                                    std::uniform_int_distribution<> disRotZ(0, z_possible_rot.size() - 1);

                                    x_deg = x_possible_rot[disRotX(gen)];
                                    y_deg = y_possible_rot[disRotY(gen)];
                                    z_deg = z_possible_rot[disRotZ(gen)];
                                }
                                //On set l'objet
                                m_grid.setObject(instance, randomX, randomY, randomZ, x_deg, y_deg, z_deg);
                                cellsDone.push_back(QVector3D(randomX, randomY, randomZ));

                                for (int j = 0; j < voisins.size(); j++) {
                                    m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).entropy++;
                                }
                            }

                    }
                }

                voisins.clear();
            } else {
                //Cas ou la cellule choisie vait un mesh, on termine l'itération et on reprend plus haut
                cellsDone.push_back(QVector3D(randomX, randomY, randomZ));
                objectSet = false;
            }
        }
    }

    m_grid.printGrid();
}

void Wfc::runWFC(int k, QVector<TileModel*> &modeles, int mode) {
    for (int i = 0; i < 30; i++) {
        initWFC(k, modeles, mode);
        bool isFull = false;
        std::random_device rd;
        std::mt19937 gen(rd());

        std::cout << "Debut de run" << std::endl;
        QVector<QVector3D> cellsDone;

        int c = 0;
        std::cout << "Dim grid : " << m_grid.getX() << " " << m_grid.getY() << " " << m_grid.getZ()
                  << " Total : " << m_grid.getX() * m_grid.getY() * m_grid.getZ() << std::endl;
        //Récupération des cellules déjà faites
        for (int x = 0; x < m_grid.getX(); x++) {
            for (int y = 0; y < m_grid.getY(); y++) {
                for (int z = 0; z < m_grid.getZ(); z++) {
                    if (m_grid.getCell(x, y, z).hasMesh) {
                        c++;
                        cellsDone.push_back(QVector3D(x, y, z));
                    }
                }
            }
        }
        std::cout<<"cellules apres ini"<<c<<std::endl;
        //Variable de limite d'itération de tentative de remplissage de grille
        int iterationLimit = m_grid.getX() * m_grid.getY() * m_grid.getZ() * 3;
        int iterationCount = 0;
        bool isEnd = false;

        while (!isFull && !isEnd) {
            iterationCount++;
            if (iterationCount > iterationLimit) {
                isEnd = true;
            }

            int maxEntropy = -1;
            QVector<QVector3D> bestEntropy;
            //Récupération de la meilleure entropie
            for (int x = 0; x < m_grid.getX(); x++) {
                for (int y = 0; y < m_grid.getY(); y++) {
                    for (int z = 0; z < m_grid.getZ(); z++) {
                        if (m_grid.getCell(x, y, z).entropy > maxEntropy && !m_grid.getCell(x, y, z).hasMesh) {
                            maxEntropy = m_grid.getCell(x, y, z).entropy;
                        }
                    }
                }
            }
            //Récupération des cellules ayant la meilleure entropie
            for (int x = 0; x < m_grid.getX(); x++) {
                for (int y = 0; y < m_grid.getY(); y++) {
                    for (int z = 0; z < m_grid.getZ(); z++) {
                        if (m_grid.getCell(x, y, z).entropy == maxEntropy && !m_grid.getCell(x, y, z).hasMesh) {
                            bestEntropy.push_back(QVector3D(x, y, z));
                        }
                    }
                }
            }

            bool isSet = false;
            //Boucle de choix de position
            while (!isSet && !bestEntropy.isEmpty()) {
                //Récupération au hasard parmi les cellules ayant la meilleure entropie
                std::uniform_int_distribution<> disEntropy(0, bestEntropy.size() - 1);
                int randomPossibility = disEntropy(gen);

                QVector3D pos = cellsDone[0];
                int randomX, randomY, randomZ;
                while (cellsDone.contains(pos)) {
                    randomX = bestEntropy[randomPossibility].x();
                    randomY = bestEntropy[randomPossibility].y();
                    randomZ = bestEntropy[randomPossibility].z();
                    pos = QVector3D(randomX, randomY, randomZ);
                }

                bestEntropy.removeAt(randomPossibility);
                //Récupération voisins
                QVector<QVector3D> voisins;
                voisins.push_back(QVector3D(std::min(randomX + 1, m_grid.getX() - 1), randomY, randomZ));
                if (!contains(voisins, QVector3D(std::max(randomX - 1, 0), randomY, randomZ)))
                    voisins.push_back(QVector3D(std::max(randomX - 1, 0), randomY, randomZ));
                if (!contains(voisins, QVector3D(randomX, std::min(randomY + 1, m_grid.getY() - 1), randomZ)))
                    voisins.push_back(QVector3D(randomX, std::min(randomY + 1, m_grid.getY() - 1), randomZ));
                if (!contains(voisins, QVector3D(randomX, std::max(randomY - 1, 0), randomZ)))
                    voisins.push_back(QVector3D(randomX, std::max(randomY - 1, 0), randomZ));
                if (!contains(voisins, QVector3D(randomX, randomY, std::min(randomZ + 1, m_grid.getZ() - 1))))
                    voisins.push_back(QVector3D(randomX, randomY, std::min(randomZ + 1, m_grid.getZ() - 1)));
                if (!contains(voisins, QVector3D(randomX, randomY, std::max(randomZ - 1, 0))))
                    voisins.push_back(QVector3D(randomX, randomY, std::max(randomZ - 1, 0)));
                removeElementFromVector(voisins, QVector3D(randomX, randomY, randomZ));
                //Récupération des règles des voisins
                QVector<QSet<int>> ruleSets;
                for (int j = 0; j < voisins.size(); j++) {
                    if (m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).hasMesh) {
                        if (mode == 0) {
                            QSet<int> set = getCorrespondingRules(QVector3D(randomX, randomY, randomZ), voisins[j],
                                                                  m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).object.tileModel());
                            ruleSets.push_back(set);
                        } else {
                            ruleSets.push_back(m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).object.tileModel()->getXMinus());
                        }
                    }
                }

                QSet<int> possibleModeles;
                //Intersection des regles pour avoir la liste des modeles possibles pour la cellule choisie
                if (!ruleSets.isEmpty()) {
                    possibleModeles = ruleSets[0];
                    for (int i = 1; i < ruleSets.size(); ++i) {
                        possibleModeles = possibleModeles.intersect(ruleSets[i]);
                    }
                }

                if (possibleModeles.isEmpty()) {
                    isSet = false;
                } else {
                    //Si il y a des modeles possibles on tire parmi ces modeles
                    QList<int> list = possibleModeles.toList();
                    std::sort(list.begin(), list.end());
                    std::uniform_int_distribution<> disModele(0, list.size() - 1);
                    int randomModel = disModele(gen);

                    bool ruleBroken = false;
                    int id;
                    //On vérifie les règles dans l'autre sens
                    for (int j = 0; j < voisins.size(); j++) {
                        if (m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).hasMesh) {
                            id = m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).object.tileModel()->id();
                            if (!(getCorrespondingRules(voisins[j], QVector3D(randomX, randomY, randomZ), modeles[list[randomModel]]).contains(id))) {
                                ruleBroken = true;
                                break;
                            }
                        }
                    }
                    //Si aucune règle n'est enfreinte on prépare les rotations
                    if (!ruleBroken) {
                        Transform transform;
                        TileModel model = *modeles[list[randomModel]];
                        TileInstance instance(new TileModel(model), transform);
                        //Récupération rotation prioritaire dans voisinage
                        float x_deg, y_deg, z_deg = 0;
                        int posPrio = findVectorPrio(voisins, cellsDone);
                        Transform transformCopy = m_grid.getCell(cellsDone[posPrio].x(), cellsDone[posPrio].y(), cellsDone[posPrio].z()).object.transform();
                        QVector3D angles = transformCopy.getRotationAngles();
                        angles=approxAngle(angles);
                        int indexX = std::abs(angles[0] / 90.0f);
                        int indexY = std::abs(angles[1] / 90.0f);
                        int indexZ = std::abs(angles[2] / 90.0f);
                        //Si rotation prioritaire autorisée on la copie sinon au hasard parmi les possibilités du modele choisi
                        if (model.getXRot()[indexX] == 1 && model.getYRot()[indexY] == 1 && model.getZRot()[indexZ] == 1) {
                            m_grid.getCell(cellsDone[posPrio].x(), cellsDone[posPrio].y(), cellsDone[posPrio].z()).object.transform().rotation().getEulerAngles(&x_deg, &y_deg, &z_deg);
                        } else {
                            QVector<bool> x_rot = model.getXRot();
                            QVector<bool> y_rot = model.getYRot();
                            QVector<bool> z_rot = model.getZRot();
                            QVector<int> x_possible_rot;
                            QVector<int> y_possible_rot;
                            QVector<int> z_possible_rot;

                            for (int j = 0; j < 4; j++) {
                                if (x_rot[j] == 1) {
                                    x_possible_rot.push_back(j * 90);
                                }
                                if (y_rot[j] == 1) {
                                    y_possible_rot.push_back(j * 90);
                                }
                                if (z_rot[j] == 1) {
                                    z_possible_rot.push_back(j * 90);
                                }
                            }

                            std::uniform_int_distribution<> disRotX(0, x_possible_rot.size() - 1);
                            std::uniform_int_distribution<> disRotY(0, y_possible_rot.size() - 1);
                            std::uniform_int_distribution<> disRotZ(0, z_possible_rot.size() - 1);
                            x_deg = x_possible_rot[disRotX(gen)];
                            y_deg = y_possible_rot[disRotY(gen)];
                            z_deg = z_possible_rot[disRotZ(gen)];
                        }
                        //On set l'objet
                        m_grid.setObject(instance, randomX, randomY, randomZ, x_deg, y_deg, z_deg);
                        cellsDone.push_back(pos);

                        isSet = true;
                        for (int j = 0; j < voisins.size(); j++) {
                            m_grid.getCell(voisins[j].x(), voisins[j].y(), voisins[j].z()).entropy++;
                        }
                        c++;
                    }
                }
            }

            if (c >= m_grid.getX() * m_grid.getY() * m_grid.getZ()) {
                std::cout << "Fin de programme normale" << std::endl;
                isFull = true;
            }
        }
        //Condition de fin
        if (isFull) {
            std::cout << "Fin apres " << i << " iterations" << std::endl;
            break;
        } else {
            m_grid.clean();
        }
    }
}




