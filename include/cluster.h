//
// Created by Yang Bo on 2020/5/11.
//

#ifndef KITSUNE_CPP_CLUSTER_H
#define KITSUNE_CPP_CLUSTER_H

#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>

/**
 *  一
Una clase auxiliar, responsable de mantener la información de asociación entre características y realizar agrupaciones jerárquicas.
 */
class Cluster {
private:
    // El tamaño del vector de instancia
    int n;

    // El número de instancias procesadas
    int num;

    // Matriz de covarianza
    double **C = nullptr;

    // Suma lineal de cada valor
    double *sum = nullptr;

    // Suma lineal de cada valor menos la media
    double *sum1 = nullptr;

    // Cada valor menos la suma de cuadrados de la media
    double *sum2 = nullptr;

    // Variables temporales durante el cálculo
    double *tmp = nullptr;

public:
    // Constructor
    Cluster(int size);

    // Incinerador de basuras
    ~Cluster();

    // Agregar una actualización de vector
    void update(const double *x);

    // Generar información cartográfica
    std::vector<std::vector<int> > *getFeatureMap(int maxSize);
};



/**
 *  
Se necesita una estructura auxiliar para la agrupación jerárquica
 */

// Representación escasa de la matriz de distancias, directamente usando triples
struct DisNode {
    int id1, id2;
    double distance;

    DisNode(int _i1, int _i2, double _d) : id1(_i1), id2(_i2), distance(_d) {}

    // Sobrecarga menos que signo
    bool operator<(const DisNode &other) const {
        return distance < other.distance;
    }
};


// Clase de nodo de dendrograma de agrupamiento jerárquico, es un árbol binario
struct ClusterNode {
    int id; // Número de nodo
    int size; //Tamaño del subárbol

    // Nodo padre
    ClusterNode *fa = nullptr;
    // Subárbol izquierdo y derecho
    ClusterNode *lson = nullptr;
    ClusterNode *rson = nullptr;

    // Constructor
    ClusterNode(int _id, int _size) : id(_id), size(_size) {}

    // Incinerador de basuras
    ~ClusterNode() {
        if (lson != nullptr)delete lson;
        if (rson != nullptr)delete rson;
    }

    // Obtener el nodo raíz del nodo actual
    ClusterNode *getRoot() {
        ClusterNode *now = this;
        while (now->fa != nullptr)now = now->fa;
        return now;
    }

    // Divida el dendrograma a través del tamaño máximo especificado (y guárdelo en el resultado.
    void getSon(std::vector<std::vector<int>> *result, int max_size);

    // Guarde todos los nodos hoja del subárbol en el resultado.
    void pushLeaf(std::vector<int> &result);
};

#endif //KITSUNE_CPP_CLUSTER_H
