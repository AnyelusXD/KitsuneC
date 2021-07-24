//
// Created by Yang Bo on 2020/5/9.
//

#ifndef KITSUNE_CPP_NEURALNET_H
#define KITSUNE_CPP_NEURALNET_H

#include <cstdio>
#include <cstring>
#include "utils.h"


/**
 *  Capa de red simple y completamente conectada
 */
class Dense {
private:
    int n_in;    // Escala de entrada

    int n_out;    // Escala de salida

    double **W = nullptr; // Peso de la conexión

    double *bias = nullptr; // Umbral

    double (*activation)(double); // Puntero de función para la función de activación

    double (*activationDerivative)(double); // Puntero de función de la derivada de la función de activación (el parámetro es el valor de la función de la función de activación)

    double learning_rate; // Tasa de aprendizaje

    double *inputValue = nullptr; //Variable temporal para guardar el valor de entrada

    double *outputValue = nullptr; // Variable temporal del valor de salida guardado

public:
    // Constructor, los parámetros son el número de neuronas de entrada, el número de neuronas de salida, la función de activación, la derivada de la función de activación, la tasa de aprendizaje (por defecto 0,1)
    Dense(int inSize, int outSize, double (*activationFunc)(double), double (*activationDerivativeFunc)(double),
          double lr = 0.1);

    ~Dense();

    //Propagación hacia adelante, el tercer parámetro indica si se deben guardar las variables temporales de los valores de entrada y salida(falso cuando solo se envía y verdadero cuando se requiere bp después de la propagación).
    void feedForward(const double *input, double *output, bool saveValue = false);

    // Retropropagar el error y guardar el error propagado a la capa anterior en g. La capacidad de g debe ser máxima (n_in, n_out
    void BackPropagation(double *g);
};


/**
 *  
Clase de autocodificador, manteniendo dos capas completamente conectadas (codificador y decodificador)
 */
class AE {
private:
    int visible_size; // El tamaño de la capa visible.

    int hidden_size; // Tamaño de capa oculta

    Dense *encoder = nullptr, *decoder = nullptr; //Red neuronal de dos capas, codificador y decodificador

    double *min_v = nullptr, *max_v = nullptr; // 0-1 valores máximos y mínimos normalizados que deben mantenerse

    double *tmp_x, *tmp_y, *tmp_z, *tmp_g; // Variables temporales

    // 0 - 1 normalizado, el resultado se almacena en tmp_x
    void normalize(const double *x);

public:
    // Constructor, el parámetro es el número de capas explícitas, capas ocultas, tasa de aprendizaje, por defecto 0.01
    AE(int v_sz, int h_sz, double _learning_rate = 0.01);

    // Incinerador de basuras
    ~AE();

    // Reconstrucción, devuelve el error medio de raíz reconstruido
    double reconstruct(const double *x);

    // Entrenamiento, devuelve el error medio de raíz reconstruido
    double train(const double *x);

};


#endif //KITSUNE_CPP_NEURALNET_H
