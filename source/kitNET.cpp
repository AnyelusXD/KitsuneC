//
// Created by Yang Bo on 2020/5/11.
//

#include "../include/kitNET.h"

void KitNET::init() {
    if (kitNetParam == nullptr) {
        fprintf(stderr, "KITNET: KitNETParam must not be null\n");
        throw -1;
    }
    // Necesita agruparse para obtener la matriz de mapeo
    if (featureMap == nullptr) {
        if (kitNetParam->cluster == nullptr) {
            fprintf(stderr, "KITNET: the Cluster object must not be null\n");
        }
        // Agrupación para obtener mapas de características
        featureMap = kitNetParam->cluster->getFeatureMap(kitNetParam->max_size);
    }

    // Inicializar el codificador automático
    ensembleLayer = new AE *[featureMap->size()];
    for (int i = 0; i < featureMap->size(); ++i) {
        ensembleLayer[i] = new AE(featureMap->at(i).size(),
                                  std::ceil(featureMap->at(i).size() * kitNetParam->ensemble_vh_rate),
                                  kitNetParam->ensemble_learning_rate);
    }
    outputLayer = new AE(featureMap->size(), std::ceil(featureMap->size() * kitNetParam->output_vh_rate),
                         kitNetParam->output_learning_rate);

    // Inicializar el búfer de parámetros de entrada desde el codificador
    ensembleInput = new double *[featureMap->size()];
    for (int i = 0; i < featureMap->size(); ++i) {
        ensembleInput[i] = new double[featureMap->at(i).size()];
    }
    outputInput = new double[featureMap->size()];

    for (auto &i : *featureMap) {
        fprintf(stderr, "[");
        for (int j : i)fprintf(stderr, "%d,", j);
        fprintf(stderr, "]\n");
    }


    delete kitNetParam;
    kitNetParam = nullptr;
}


// Incinerador de basuras
KitNET::~KitNET() {
    delete kitNetParam; // Si es nulo, eliminar nulo no tiene ningún efecto, así que simplemente elimínelo directamente
    for (int i = 0; i < featureMap->size(); ++i) {
        delete ensembleLayer[i];
        delete[] ensembleInput[i];
    }
    delete outputLayer;
    delete[] outputInput;
    delete featureMap;
}

double KitNET::train(const double *x) {
    if (featureMap == nullptr) { // Si el mapa de características no se ha inicializado
        --kitNetParam->fm_train_num;
        // Actualizar los valores mantenidos en el clúster
        kitNetParam->cluster->update(x);
        // Si el número de mapas de funciones de entrenamiento alcanza el valor establecido, inicialice el codificador automático
        if (kitNetParam->fm_train_num == 0)init();
        return 0;
    } else {// Autoencoder de tren
        for (int i = 0; i < featureMap->size(); ++i) {
            // Copie el vector de características correspondiente al búfer
            for (int j = 0; j < featureMap->at(i).size(); ++j) {
                ensembleInput[i][j] = x[featureMap->at(i).at(j)];
            }
            outputInput[i] = ensembleLayer[i]->train(ensembleInput[i]);
        }
        // Entrene la capa de salida, devuelva el error de reconstrucción
        return outputLayer->train(outputInput);
    }
}

double KitNET::execute(const double *x) {
    if (featureMap == nullptr) { // Si el mapa de características no se ha inicializado
        fprintf(stderr, "KitNET: the feature map is not initialized!!\n");
        throw -1;
    }
    for (int i = 0; i < featureMap->size(); ++i) {
        // Copie el vector de características correspondiente al búfer
        for (int j = 0; j < featureMap->at(i).size(); ++j) {
            ensembleInput[i][j] = x[featureMap->at(i).at(j)];
        }
        // Ejecute este pequeño codificador automático y guarde el error de reconstrucción como entrada de la capa de salida
        outputInput[i] = ensembleLayer[i]->reconstruct(ensembleInput[i]);
    }
    return outputLayer->reconstruct(outputInput);
}
