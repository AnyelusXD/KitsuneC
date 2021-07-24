//
// Created by Yang Bo on 2020/5/11.
//

#ifndef KITSUNE_CPP_KITNET_H
#define KITSUNE_CPP_KITNET_H

#include <vector>
#include "neuralnet.h"
#include "cluster.h"


/**
*  KitNET crea la capa de integración y los parámetros necesarios cuando la capa de salida es
*/
struct KitNETParam {
    int max_size; // Tamaño máximo del codificador de capa integrado

    // La cantidad de muestras necesarias para entrenar el mapa de características.
    int fm_train_num = 0;

    // La proporción de la capa oculta del autocodificador de la capa integrada
    double ensemble_vh_rate;

    // El codificador automático de la capa de salida muestra la relación de capa oculta
    double output_vh_rate;

    // Tasa de aprendizaje de la capa de integración
    double ensemble_learning_rate;

    // Tasa de aprendizaje de la capa de salida
    double output_learning_rate;

    // Clase auxiliar para agrupamiento
    Cluster *cluster = nullptr;

    // Incinerador de basuras
    ~KitNETParam() {
        if (cluster != nullptr)delete cluster;
    }
};


/**
 *  KitNET Clase, principalmente a través de clustering, autoencoder implementa el algoritmo KitNET
 *
 */

class KitNET {
private:
    // Mapeo de características, guarde el codificador automático al que se asigna cada elemento del vector de instancia de característica.
    std::vector<std::vector<int> > *featureMap = nullptr;

    // Autocodificador de capa integrado
    AE **ensembleLayer = nullptr;

    // Autoencoder de la capa de salida
    AE *outputLayer = nullptr;

    // El vector de entrada de la capa de integración
    double **ensembleInput = nullptr;

    // Vector de entrada de la capa de salida
    double *outputInput = nullptr;

    // Parámetros necesarios para inicializar el codificador automático
    KitNETParam *kitNetParam = nullptr;

    // Inicializar KitNET, inicializar según parámetros característicos, etc.
    void init();

public:

    // Hay dos tipos de constructores, uno es para proporcionar directamente el mapeo de características. El otro es para entrenar el mapeo de características basado en parámetros, que se realizará después del entrenamiento

    // Constructor 1, los parámetros son :
    // 1. Puntero de matriz para mapeo de características
    // 2. Capa de integración de puntero de matriz mapeada autocodificador capa explícita / proporción de capa oculta 3. capa de salida autocodificador capa explícita / proporción de capa oculta, (todas predeterminadas 0,75)
    // 4. La tasa de aprendizaje de la capa de integración 5. La tasa de aprendizaje de la capa de salida (todas predeterminadas 0.1)
    KitNET(std::vector<std::vector<int> > *fm, double ensemble_vh_rate = 0.75, double output_vh_rate = 0.75,
           double ensemble_learning_rate = 0.1, double output_learning_rate = 0.1) {
        featureMap = fm;
        kitNetParam = new KitNETParam;
        kitNetParam->ensemble_learning_rate = ensemble_learning_rate;
        kitNetParam->ensemble_vh_rate = ensemble_vh_rate;
        kitNetParam->output_vh_rate = output_vh_rate;
        kitNetParam->output_learning_rate = output_learning_rate;
        init(); // Crea un autocodificador directamente
    }


    // Constructor 2, los parámetros son :
    // 1. Ingrese el tamaño de la instancia
    // 2. La mayor escala de cada codificador automático en la capa de integración,
    // 3. La cantidad de instancias necesarias para entrenar el mapa de características.
    // 4. La proporción de la capa explícita / oculta del autocodificador de capa integrada 5. La proporción de la capa explícita / capa oculta del autocodificador de la capa de salida, (todas predeterminadas 0, 75)
    // 6. La tasa de aprendizaje de la capa de integración 7. La tasa de aprendizaje de la capa de salida(todas predeterminadas 0.1)
    KitNET(int n, int maxAE, int fm_train_num, double ensemble_vh_rate = 0.75, double output_vh_rate = 0.75,
           double ensemble_learning_rate = 0.1, double output_learning_rate = 0.1) {
        kitNetParam = new KitNETParam;
        kitNetParam->ensemble_learning_rate = ensemble_learning_rate;
        kitNetParam->ensemble_vh_rate = ensemble_vh_rate;
        kitNetParam->output_vh_rate = output_vh_rate;
        kitNetParam->output_learning_rate = output_learning_rate;
        kitNetParam->max_size = maxAE;
        kitNetParam->fm_train_num = fm_train_num;
        kitNetParam->cluster = new Cluster(n);
    }


    // Incinerador de basuras
    ~KitNET();

    // Entrenando, devuelve el error de reconstrucción. Si estás entrenando el módulo FM, devuelve
    double train(const double *x);

    // Propagación del término anterior, devuelve el error de reconstrucción de los datos actuales
    double execute(const double *x);


};


#endif //KITSUNE_CPP_KITNET_H
