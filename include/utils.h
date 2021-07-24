//
// Created by Yang Bo on 2020/5/9.
//

#ifndef KITSUNE_CPP_UTILS_H
#define KITSUNE_CPP_UTILS_H

/**
 *  一Algunas herramientas o funciones auxiliares
 */

#include <cstdio>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>


// Convierta el archivo pcap a tsv y devuelva el puntero del archivo tsv
FILE *pcap2tcv(const char *);


/*
 *  TvsReader, responsable de leer tsv, clases de formato csv, inicializado con nombre de archivo, para cada fila se puede leer por el id de la columna
 */
class TsvReader {
private:
    std::FILE *fp;
    char *buffer;
    std::vector<int> id; // La posición de la i-ésima columna en el búfer actual (comenzando desde 0)
    char delimitor;  // Separador de corriente

    // Tamaño de búfer constante
    static const int BufferSize = 3000;
public:
    // Constructor, los parámetros son el nombre del archivo y el separador, el predeterminado es el archivo tsv(el separador es '\ t')
    TsvReader(const char *filename, char d = '\t') {
        fp = nullptr;
        fp = std::fopen(filename, "r");
        if (fp == nullptr) {
            std::fprintf(stderr, "\nTsvReader: File name is invalid!\n");
            throw -1;
        }
        delimitor = d;
        buffer = new char[BufferSize];
    }

    //Constructor, los parámetros son el puntero del archivo y el separador, el predeterminado es el archivo tsv (el separador es '\ t')
    TsvReader(FILE *_fp, char d = '\t') {
        fp = _fp;
        if (fp == nullptr) {
            std::fprintf(stderr, "\nTsvReader: File pointer is invalid!\n");
            throw -1;
        }
        delimitor = d;
        buffer = new char[BufferSize];
    }

    // Leer y preprocesar la siguiente fila, devolver el número de columnas de la fila actual. Si se lee la última fila, devolver 0
    int nextLine();

    // Convierta la columna de la columna en una cadena y regrese
    std::string getString(int col);

    // Convierta la columna col en un int y regrese
    inline int getInt(int col) {
        int ans = 0;
        int now = id[col];
        while (buffer[now] >= '0' && buffer[now] <= '9')
            ans = (ans << 3) + (ans << 1) + buffer[now++] - '0';
        return ans;
    }

    // Convierte la columna de la columna en un doble y vuelve
    inline double getDouble(int col) {
        return strtod(buffer + id[col], nullptr);
    }

    // Determine si la columna col tiene un valor
    inline bool hasValue(int col) {
        int now = id[col];
        return buffer[now] != delimitor && buffer[now] != '\r' && buffer[now] != '\n' && buffer[now] != '\0';
    }

    // Destructor, libera espacio
    ~TsvReader() {
        if (fp != nullptr) std::fclose(fp);
        delete[] buffer;
    }

};

/**
 *  Clase para generar archivos csv / tsv
 */
class TsvWriter {
private:
    // Puntero de archivo mantenido
    FILE *fp = nullptr;
    // Delimitador
    char delimitor;
public:
    // Constructor, los parámetros son nombre de archivo, separador
    TsvWriter(const char *filename, char d = '\t') {
        // Inicializar el puntero del archivo
        fp = std::fopen(filename, "w");
        if (fp == nullptr) {
            std::fprintf(stderr, "\nTsvWriter: file open Error!\n");
            throw -1;
        }

        delimitor = d;
    }

    void write(const double *p, int n) {
        fprintf(fp, "%.10f", p[0]);
        for (int i = 1; i < n; ++i)fprintf(fp, "%c%.16f", delimitor, p[i]);
        fprintf(fp, "\n");
    }

    ~TsvWriter() { std::fclose(fp); }
};


/**
 *  一Serie de funciones de activación comunes
 */

inline double sigmoid(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}

inline double sigmoidDerivative(double fx) {
    return fx * (1 - fx);
}

inline double ReLU(double x) {
    return x < 0 ? 0 : x;
}

inline double pReLU(double x) {
    const static double alpha = 0.01;
    return x < 0 ? alpha * x : x;
}

inline double ELU(double x) {
    const static double alpha = 0.01;
    return x < 0 ? alpha * (std::exp(x) - 1) : x;
}


/**
 * 一Índice de evaluación de regresión
 */

//Error cuadrático medio
inline double MSE(const double *a, const double *b, int n) {
    if (n <= 0)return 0;
    double sum = 0;
    for (int i = 0; i < n; ++i) {
        double tmp = a[i] - b[i];
        sum += tmp * tmp;
    }
    return sum / n;
}

// Error cuadrático medio
inline double RMSE(const double *a, const double *b, int n) {
    if (n <= 0)return 0;
    double sum = 0;
    for (int i = 0; i < n; ++i) {
        double tmp = a[i] - b[i];
        sum += tmp * tmp;
    }
    return std::sqrt(sum / n);
}

// Error absoluto medio
inline double MAE(const double *a, const double *b, int n) {
    if (n <= 0)return 0;
    double sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += std::fabs(a[i] - b[i]);
    }
    return sum / n;
}

/**
 *  Función para generar datos aleatorios
 */

// Distribuidos equitativamente
inline double rand_uniform(double _min, double _max) {
    static bool seed = false;
    if(!seed){// Establecer la semilla de número aleatorio una vez
        std::srand(std::time(NULL));
        seed = true;
    }
    return rand() / (RAND_MAX + 0.1) * (_max - _min) + _min;
}

#endif //KITSUNE_CPP_UTILS_H
