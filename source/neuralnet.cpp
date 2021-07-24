//
// Created by Yang Bo on 2020/5/9.
//

#include "../include/neuralnet.h"


Dense::Dense(int inSize, int outSize, double (*activationFunc)(double), double (*activationDerivativeFunc)(double),
             double lr) {
    n_in = inSize;
    n_out = outSize;
    activation = activationFunc;
    activationDerivative = activationDerivativeFunc;
    learning_rate = lr;
    inputValue = new double[n_in];
    outputValue = new double[n_out];
    bias = new double[n_out];
    W = new double *[n_in];// n_en fila, n_out columna
    for (int i = 0; i < n_in; ++i)W[i] = new double[n_out];

    double val = 1.0 / n_out;
    // Pesos de inicialización distribuidos uniformemente
    for (int i = 0; i < n_in; ++i) {
        for (int j = 0; j < n_out; ++j)W[i][j] = rand_uniform(-val, val);
    }
    for (int i = 0; i < n_out; ++i)bias[i] = 0;
}

Dense::~Dense() {
    delete[] bias;
    delete[] inputValue;
    delete[] outputValue;
    for (int i = 0; i < n_in; ++i)delete[] W[i];
    delete[] W;
}

void Dense::feedForward(const double *input, double *output, bool saveValue) {
    for (int i = 0; i < n_out; ++i) {
        output[i] = bias[i];
        for (int j = 0; j < n_in; ++j) {
            output[i] += W[j][i] * input[j];
        }
        output[i] = activation(output[i]);
    }
    if (saveValue) {
        std::memcpy(inputValue, input, sizeof(double) * n_in);
        std::memcpy(outputValue, output, sizeof(double) * n_out);
    }
}

// SGD
void Dense::BackPropagation(double *g) {
    for (int i = 0; i < n_out; ++i)outputValue[i] = g[i] * activationDerivative(outputValue[i]);

    // Calcule el error propagado a la capa superior.
    for (int i = 0; i < n_in; ++i) {
        g[i] = 0;
        for (int j = 0; j < n_out; ++j) {
            g[i] += W[i][j] * outputValue[j];
        }
    }

    // Actualice el umbral, por cierto, multiplique learning_rate por el guardado, no es necesario calcular al actualizar el peso
    for (int i = 0; i < n_out; ++i) {
        outputValue[i] *= learning_rate;
        bias[i] += outputValue[i];
    }

    // Actualizar peso
    for (int i = 0; i < n_in; ++i) {
        for (int j = 0; j < n_out; ++j) {
            W[i][j] += inputValue[i] * outputValue[j];
        }
    }
}


// Constructor, el parámetro es el número de capas explícitas y ocultas.

AE::AE(int v_sz, int h_sz, double _learning_rate) {
    visible_size = v_sz;
    hidden_size = h_sz;

    // Inicializa dos capas de redes neuronales, ambas usan la función de activación sigmoidea
    encoder = new Dense(visible_size, hidden_size, sigmoid, sigmoidDerivative, _learning_rate);
    decoder = new Dense(hidden_size, visible_size, sigmoid, sigmoidDerivative, _learning_rate);

    // Inicializar una matriz de variables temporales
    tmp_x = new double[visible_size];
    tmp_z = new double[visible_size];
    tmp_y = new double[hidden_size];
    // tmp_g es el búfer utilizado para propagar el gradiente, por lo que el tamaño es el valor máximo de cada capa
    tmp_g = new double[std::max(hidden_size, visible_size)];

    // Inicializar la matriz requerida para la normalización
    max_v = new double[visible_size];
    min_v = new double[visible_size];
    for (int i = 0; i < visible_size; ++i) {
        min_v[i] = 1e20;
        max_v[i] = -1e20;
    }
}

// Incinerador de basuras
AE::~AE() {
    delete encoder;
    delete decoder;
    delete[] tmp_x;
    delete[] tmp_y;
    delete[] tmp_z;
    delete[] max_v;
    delete[] min_v;
}


// Reconstruir, devolver el valor reconstruido
double AE::reconstruct(const double *x) {
    normalize(x); // Primero normalice y guarde en tmp_x

    encoder->feedForward(tmp_x, tmp_y); // Codificación, almacenada en tmp_y

    decoder->feedForward(tmp_y, tmp_z); // Decodificar y guardar en tmp_z

    return RMSE(tmp_x, tmp_z, visible_size);
}

// capacitación
double AE::train(const double *x) {
    normalize(x); // Regularización 0-1, almacenada en tmp_x
    // Ejecútelo hacia adelante, establezca el parámetro saveValue en verdadero y prepárese para propagar el error de regreso
    encoder->feedForward(tmp_x, tmp_y, true);
    decoder->feedForward(tmp_y, tmp_z, true);

    // El error se almacena en tmp_g
    for (int i = 0; i < visible_size; ++i)tmp_g[i] = tmp_x[i] - tmp_z[i];
    // Error de retropropagación
    decoder->BackPropagation(tmp_g);
    encoder->BackPropagation(tmp_g);

    return RMSE(tmp_x, tmp_z, visible_size);
}

// 0-1 normalizado, el resultado se almacena en tmp_x
void AE::normalize(const double *x) {
    for (int i = 0; i < visible_size; ++i) {
        min_v[i] = std::min(x[i], min_v[i]);
        max_v[i] = std::max(x[i], max_v[i]);
        tmp_x[i] = (x[i] - min_v[i]) / (max_v[i] - min_v[i] + 1e-13);
    }
}
