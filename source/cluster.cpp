//
// Created by Yang Bo on 2020/5/11.
//

#include "../include/cluster.h"

Cluster::Cluster(int size) {
    n = size;
    sum = new double[n];
    sum1 = new double[n];
    sum2 = new double[n];
    num = 0;
    C = new double *[n];
    for (int i = 0; i < n; ++i)C[i] = new double[n];
    tmp = new double[n];

    // Inicializar variables
    for (int i = 0; i < n; ++i)sum[i] = 0;
    for (int i = 0; i < n; ++i)sum1[i] = 0;
    for (int i = 0; i < n; ++i)sum2[i] = 0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)C[i][j] = 0;
}

void Cluster::update(const double *x) {
    ++num;
    for (int i = 0; i < n; ++i) {
        sum[i] += x[i];
        tmp[i] = x[i] - sum[i] / num;
        sum1[i] += tmp[i];
        sum2[i] += tmp[i] * tmp[i];
    }
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            C[i][j] += tmp[i] * tmp[j];
        }
    }
}

std::vector<std::vector<int> > *Cluster::getFeatureMap(int maxSize) {
    // Obtenga cada valor de cada característica menos la raíz cuadrada de la suma de cuadrados de la media (calcule el denominador del coeficiente de correlación)
    // Guárdelo en tmp.
    for (int i = 0; i < n; ++i) {
        if (sum2[i] <= 0)tmp[i] = 0;
        else tmp[i] = std::sqrt(sum2[i]);
    }
    std::vector<DisNode> dis;
    dis.clear();
    // Genere una matriz de distancia(indicada por triples) y solo obtenga la mitad de la matriz
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            double l = tmp[i] * tmp[j];
            if (l <= 0)l = 1e-20;
            l = 1 - C[i][j] / l;
            if (l < 0)l = 0;
            dis.emplace_back(i, j, l);
        }
    }
    // Ordene la matriz por distancia y combine los dos grupos con la distancia más pequeña cada vez
    std::sort(dis.begin(), dis.end());

    std::vector<ClusterNode*> leaf; // Nodo hoja
    leaf.clear();
    for (int i = 0; i < n; ++i)leaf.push_back(new ClusterNode(i, 1));
    int now_id = n;
    for (auto edge: dis) {
        ClusterNode *root1 = leaf[edge.id1]->getRoot();
        ClusterNode *root2 = leaf[edge.id2]->getRoot();
        // Cuando los dos no están en el mismo clúster, se fusionan para generar un nuevo nodo
        if (root1 != root2) {
            auto *root = new ClusterNode(now_id++, root1->size + root2->size);
            root1->fa = root;
            root2->fa = root;
            root->lson = root1;
            root->rson = root2;
        }
    }
    // Después de la fusión, todo debe estar en un árbol, es decir, se obtiene un dendrograma.
    ClusterNode *root = leaf.back()->getRoot();
    // Corte el dendrograma en varios grupos, cada grupo no exceda maxSize;
    auto *ans = new std::vector<std::vector<int> >();
    root->getSon(ans, maxSize);
    // Libera la memoria del diagrama de árbol
    delete root;
    return ans;
}

Cluster::~Cluster() {
    delete[] sum;
    delete[] sum1;
    delete[] sum2;
    for (int i = 0; i < n; ++i)delete[] C[i];
    delete[] C;
    delete[] tmp;
}

void ClusterNode::getSon(std::vector<std::vector<int>> *result, int max_size) {
    if (size <= max_size) { // Si la cantidad actual de subárboles cumple con los requisitos, colóquelos todos en un grupo directamente
        std::vector<int> ans;
        ans.clear();
        pushLeaf(ans);
        result->push_back(ans);
    }else{ // De lo contrario, se divide en dos grupos y se juzga de forma recursiva.
        // Subárbol izquierdo
        lson->getSon(result,max_size);
        // Subárbol derecho
        rson->getSon(result,max_size);
    }
}

void ClusterNode::pushLeaf(std::vector<int> &result) {
    if (lson == nullptr && rson == nullptr) {//leaf
        result.push_back(id);
    } else {
        if (lson != nullptr)lson->pushLeaf(result);
        if (rson != nullptr)rson->pushLeaf(result);
    }
}
