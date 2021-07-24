//
// Created by Yang Bo on 2020/5/9.
//

#include "../include/netStat.h"


// Insertar nuevo elemento
void QueueFixed::insert(double x) {
    array[now_index++] = x;
    if (now_index >= QueueCapacity)now_index = 0;
    ++now_size;
}

// Expande la cola y devuelve el número de elementos de la matriz.
int QueueFixed::unroll(double *ans) {
    if (now_size >= QueueCapacity) {// Si está lleno, now_index es el primero al que se apunta
        for (int i = 0, j = now_index; i < QueueCapacity; ++i) {
            ans[i] = array[j];
            ++j;
            if (j >= QueueCapacity)j = 0;
        }
        return QueueCapacity;
    } else {
        for (int i = 0; i < now_index; ++i)ans[i] = array[i];
        return now_size;
    }
}

// Devuelve el último elemento que entró en la cola.
double QueueFixed::getLast() {
    if (now_index == 0)return array[QueueCapacity - 1];
    return array[now_index - 1];
}

// Utilice la interpolación lagrangiana para predecir el próximo valor
double Extrapolator::predict(double t) {
    // Expandir el argumento en una matriz
    int sz = tQ.unroll(tArr);
    if (sz < 2) { // Menos de 2, impredecible
        if (sz == 0)return 0;
        else return vQ.getLast();
    }

    double diff_sum = 0;
    for (int i = 1; i < sz; ++i)diff_sum += tArr[i] - tArr[i - 1];
    // Si la diferencia de tiempo entre la hora actual y la anterior es más de diez veces la diferencia de tiempo promedio, será impredecible y el último resultado de la cola se devolverá directamente.
    if (diff_sum / (sz - 1) * 10 < (t - tQ.getLast()))
        return vQ.getLast();

    // Expandir el valor de la función
    vQ.unroll(vArr);

    // Interpolación lagrangiana
    // Fórmula de interpolación de Lagrange, L(tp) = sum_{ i = 1 }^ {n + 1} {y_i* l_i(tp)}
    // Donde l_i(tp) = \ frac{ \ prod_ {j = 1 y j!= I} (tp - x_j) } {\ prod_{ j = 1 y j!= I } (x_i - x_j)}
    double ans = 0;
    for (int i = 0; i < sz; ++i) {
        double y = vArr[i];
        for (int j = 0; j < sz; ++j) {
            if (i == j)continue;
            y *= (t - tArr[j]) / (tArr[i] - tArr[j] + 1e-20);
        }
        ans += y;
    }
    return ans;
}

// Constructor de incStat
IncStat::IncStat(const std::string &id, std::vector<double> *_lambdas, double init_time, bool isTypediff) {
    ID = id;
    lambdas = _lambdas;
    isTypeDiff = isTypediff;
    lastTimestamp = init_time;
    mean_valid = var_valid = std_valid = false;
    auto size = lambdas->size();
    // Asignar memoria
    CF1 = new double[size];
    CF2 = new double[size];
    w = new double[size];
    cur_std = new double[size];
    cur_mean = new double[size];
    cur_var = new double[size];
    // inicialización
    for (int i = 0; i < size; ++i)CF1[i] = 0;
    for (int i = 0; i < size; ++i)CF2[i] = 0;
    for (int i = 0; i < size; ++i)w[i] = 1e-20;// Evita la división por 0
}

// incStat Incinerador de basuras
IncStat::~IncStat() {
    for (auto v:covs) {// Principalmente para liberar el recuerdo de la relación entre las dos corrientes mantenidas
        // Esta instancia apuntará a múltiples punteros de clase, por lo que se mantiene un refNum, y cuando se reduce a 0, se elimina
        if ((--v->refNum) == 0)delete v;
    }
    delete[] CF1;
    delete[] CF2;
    delete[] w;
    delete[] cur_std;
    delete[] cur_mean;
    delete[] cur_var;
}

// La secuencia inserta nuevas estadísticas.
void IncStat::insert(double v, double t) {
    // Si isTypeDiff está configurado, use la diferencia de tiempo como información estadística
    if (isTypeDiff) {
        double dif = t - lastTimestamp;
        v = dif > 0 ? dif : 0;
    }

    // Primero decae
    processDecay(t);

    // Actualizar con v
    for (size_t i = 0; i < lambdas->size(); ++i)CF1[i] += v;
    for (size_t i = 0; i < lambdas->size(); ++i)CF2[i] += v * v;
    for (size_t i = 0; i < lambdas->size(); ++i)++w[i];

    // La media, la varianza y la desviación estándar no se calculan primero y se calcularán más tarde.
    mean_valid = var_valid = std_valid = false;

}

// Realice la atenuación, el parámetro es la marca de tiempo actual
void IncStat::processDecay(double timestamp) {
    double diff = timestamp - lastTimestamp;
    if (diff > 0) {
        for (size_t i = 0; i < lambdas->size(); ++i) {
            // Calcular el factor de atenuación
            double factor = std::pow(2.0, -lambdas->at(i) * diff);
            CF1[i] *= factor;
            CF2[i] *= factor;
            w[i] *= factor;
        }
        lastTimestamp = timestamp;
    }
}

// Calcule la media
void IncStat::calMean() {
    if (!mean_valid) { // Calcular cuando sea necesario
        mean_valid = true;
        for (size_t i = 0; i < lambdas->size(); ++i)
            cur_mean[i] = CF1[i] / w[i];
    }
}

// Calcular la varianza
void IncStat::calVar() {
    if (!var_valid) {
        var_valid = true;
        calMean(); // El cálculo requiere la media, actualice la media primero
        for (size_t i = 0; i < lambdas->size(); ++i)
            cur_var[i] = fabs(CF2[i] / w[i] - cur_mean[i] * cur_mean[i]);
    }
}

// Calcule la desviación estándar
void IncStat::calStd() {
    if (!std_valid) {
        std_valid = true;
        calVar(); // El cálculo requiere varianza, primero calcule
        for (size_t i = 0; i < lambdas->size(); ++i)
            cur_std[i] = std::sqrt(cur_var[i]);
    }
}

// Obtenga todas las estadísticas unidimensionales(peso, media, varianza)
int IncStat::getAll1DStats(double *result) {
    calMean();
    calVar();
    int offset = 0;
    for (size_t i = 0; i < lambdas->size(); ++i)result[offset++] = (w[i]);
    for (size_t i = 0; i < lambdas->size(); ++i)result[offset++] = (cur_mean[i]);
    for (size_t i = 0; i < lambdas->size(); ++i)result[offset++] = (cur_var[i]);
    return offset;
}


//Actualice estadísticas como la covarianza de estos dos flujos.
//Solo se puede llamar después de que se actualice una de las dos secuencias y, a continuación, el parámetro es el ID de la secuencia actualizada y la v y la t utilizada para la actualización de la secuencia actualizada.
//Es decir, después de actualizar uno de los métodos de inserción de flujo, este método se llama inmediatamente para actualizar las estadísticas relevantes.
void IncStatCov::updateCov(const std::string &ID, double v, double t) {
    // Primero la atenuación
    processDecay(t);

    // Actualizar la media de las dos corrientes
    incS1->calMean();
    incS2->calMean();

    if (ID == incS1->ID) { // Si es la primera actualización que circula
        // Actualizar la información mantenida por el primer método de extrapolación de flujo
        ex1.insert(t, v);
        // Obtenga el valor actualizado de la predicción de la segunda transmisión
        double v_other = ex1.predict(t);
        for (size_t i = 0; i < lambdas->size(); ++i) {
            CF3[i] += (v - incS1->cur_mean[i]) * (v_other - incS2->cur_mean[i]);
        }
    } else {// El valor actualizado de la segunda secuencia
        // Actualizar la información mantenida por el método de extrapolación del segundo flujo
        ex2.insert(t, v);
        // Obtenga el valor previsto de la primera transmisión
        double v_other = ex2.predict(t);
        // Actualizar la parte del numerador de la covarianza (CF3)
        for (size_t i = 0; i < lambdas->size(); ++i) {
            CF3[i] += (v_other - incS1->cur_mean[i]) * (v - incS2->cur_mean[i]);
        }
    }
    // Actualizar peso
    for (size_t i = 0; i < lambdas->size(); ++i) ++w3[i];
}

// Realizar una función de decaimiento
void IncStatCov::processDecay(double t) {
    double diff = t - lastTimestamp;
    if (diff > 0) {
        for (size_t i = 0; i < lambdas->size(); ++i) {
            double factor = std::pow(2.0, -lambdas->at(i) * diff);
            CF3[i] *= factor;
            w3[i] *= factor;
        }
        lastTimestamp = t;
    }
}

// Calcule el radio de las dos corrientes (raíz cuadrada de la suma de las varianzas)
int IncStatCov::getRadius(double *result) {
    incS1->calVar();
    incS2->calVar();
    for (size_t i = 0; i < lambdas->size(); ++i) {
        result[i] = (std::sqrt(incS1->cur_var[i] + incS2->cur_var[i]));
    }
    return lambdas->size();
}

// Calcule la raíz cuadrada de la suma de los cuadrados medios de dos corrientes
int IncStatCov::getMagnitude(double *result) {
    incS1->calMean();
    incS2->calMean();
    for (size_t i = 0; i < lambdas->size(); ++i) {
        double mean1 = incS1->cur_mean[i];
        double mean2 = incS2->cur_mean[i];
        result[i] = (std::sqrt(mean1 * mean1 + mean2 * mean2));
    }
    return lambdas->size();
}

// Calcule la covarianza de dos corrientes
int IncStatCov::getCov(double *result) {
    for (size_t i = 0; i < lambdas->size(); ++i)
        result[i] = (CF3[i] / w3[i]);
    return lambdas->size();
}

// Calcule el coeficiente de correlación de dos corrientes
int IncStatCov::getPcc(double *result) {
    incS1->calStd();
    incS2->calStd();
    for (size_t i = 0; i < lambdas->size(); ++i) {
        double ss = incS1->cur_std[i] * incS2->cur_std[i];
        if (ss < 1e-20) result[i] = 0;
        else result[i] = (CF3[i] / (w3[i] * ss));
    }
    return lambdas->size();
}

//Obtenga toda la información estadística bidimensional[radio, magnitud, cov, pcc], devuelva el número agregado a la matriz
int IncStatCov::getAll2DStats(double *result) {
    int offset = getRadius(result);
    offset += getMagnitude(result + offset);
    offset += getCov(result + offset);
    return offset + getPcc(result + offset);
}


// Actualiza la información unidimensional y bidimensional de la transmisión especificada y devuelve un [peso, media, estándar] y bidimensional [radio, magnitud, cov, pcc].
// Los parámetros son: el ID de la transmisión, la marca de tiempo, los datos estadísticos, la referencia del resultado devuelto y el último si se establece en verdadero, la marca de tiempo se utiliza como datos estadísticos

int IncStatDB::updateGet1DStats(const std::string &ID, double t, double v, double *result, bool isTypeDiff) {
    auto it = stats.find(ID);
    if (it == stats.end()) { // Si no lo encuentra, genere una nueva transmisión
        auto *incStat = new IncStat(ID, lambdas, t, isTypeDiff);
        auto ret = stats.insert(std::make_pair(ID, incStat));
        it = ret.first;
    }
    // it->second Estadísticas de la corriente apuntada ahora
    it->second->insert(v, t);
    return it->second->getAll1DStats(result);
}


// Actualiza la información bidimensional de la secuencia especificada y agrega [radio, magnitud, cov, pcc] al resultado
// Los parámetros son: el ID del primer flujo, el ID del segundo flujo, las estadísticas del primer flujo, la marca de tiempo, el puntero a la matriz de resultados,
// Devuelve el número de datos agregados a la matriz de resultados

int IncStatDB::updateGet2DStats(const std::string &ID1, const std::string &ID2, double t1, double v1,
                                double *result, bool isTypediff) {

    // Obtener dos flujos, generar uno nuevo si no se encuentra
    auto it1 = stats.find(ID1);
    if (it1 == stats.end()) { // Si no lo encuentra, genere una nueva secuencia
        auto *incStat1 = new IncStat(ID1, lambdas, t1, isTypediff);
        auto ret1 = stats.insert(std::make_pair(ID1, incStat1));
        it1 = ret1.first;
    }
    auto it2 = stats.find(ID2);
    if (it2 == stats.end()) { // Si no lo encuentra, genere una nueva secuencia
        auto *incStat2 = new IncStat(ID2, lambdas, t1, isTypediff);
        auto ret2 = stats.insert(std::make_pair(ID2, incStat2));
        it2 = ret2.first;
    }

    // Obtenga la relación entre dos transmisiones y actualice todas las demás relaciones de transmisión relacionadas con ID1 al mismo tiempo
    IncStatCov *incStatCov = nullptr;
    for (auto v:it1->second->covs) {
        v->updateCov(ID1, v1, t1);
        // Mientras actualiza, busque transmisiones relacionadas con ID2
        if (incStatCov == nullptr && (v->incS1->ID == ID2 || v->incS2->ID == ID2))
            incStatCov = v;
    }

    // Si no lo encuentra, genere una nueva relación entre las corrientes
    if (incStatCov == nullptr) {
        incStatCov = new IncStatCov(it1->second, it2->second, lambdas, t1);
        incStatCov->refNum = 2;
        // Ambos flujos guardan esta referencia, y el número de referencias se juzgará cuando se destruya, y se eliminará solo cuando sea 0
        it1->second->covs.push_back(incStatCov);
        it2->second->covs.push_back(incStatCov);
        incStatCov->updateCov(ID1, v1, t1);
    }

    // Obtener estadísticas entre dos transmisiones
    return incStatCov->getAll2DStats(result);
}


// Constructor, los parámetros son lambdas
NetStat::NetStat(const std::vector<double> &l) {
    // Inicialice la información de cuatro flujos mantenidos y pase el puntero de la lista de la ventana de tiempo.
    lambdas = std::vector<double>(l);
    HT_jit = new IncStatDB(&lambdas);
    HT_Hp = new IncStatDB(&lambdas);
    HT_MI = new IncStatDB(&lambdas);
    HT_H = new IncStatDB(&lambdas);
}

// Sin constructor de parámetros, use lambdas predeterminadas
NetStat::NetStat() {
    lambdas = std::vector<double>({5, 3, 1, 0.1, 0.01});
    HT_jit = new IncStatDB(&lambdas);
    HT_Hp = new IncStatDB(&lambdas);
    HT_MI = new IncStatDB(&lambdas);
    HT_H = new IncStatDB(&lambdas);
}

// La función de llamada principal, pasa la información de un paquete y devuelve el vector estadístico correspondiente
// Los parámetros son: MAC de origen, MCA de destino, IP de origen, tipo de protocolo IP, IP de destino, tipo de protocolo IP de destino, tamaño del paquete, marca de tiempo del paquete
int NetStat::updateAndGetStats(const std::string &srcMAC, const std::string &dstMAC,
                               const std::string &srcIP, const std::string &srcProtocol,
                               const std::string &dstIP, const std::string &dstProtocol,
                               double datagramSize, double timestamp, double *result) {

    int offset = 0; // Desplazamiento de la matriz(el número de colocados actualmente)

    // MAC.IP: Estadísticas de origen de host MAC e relación IP y ancho de banda
    offset += HT_MI->updateGet1DStats(srcMAC + srcIP, timestamp, datagramSize, result);

    // Host-Host BW: Estadísticas del flujo de envío del host IP de origen (relación unidimensional), relación bidimensional entre el comportamiento de envío del host IP de origen y el host IP de destino
    offset += HT_H->updateGet1D2DStats(srcIP, dstIP, timestamp, datagramSize, result + offset);

    // Host-Host Jitter: Fluctuación entre el host y el host
    offset += HT_jit->updateGet1DStats(srcIP + dstIP, timestamp, 0, result + offset, true);

    // Host-Host BW: Estadísticas del flujo de envío del puerto IP de origen (relación unidimensional) Relación del comportamiento de envío entre el puerto IP de origen y el puerto IP de destino (relación bidimensional)
    // Si no es un paquete tcp / udp, deje que la dirección mac sea el valor clave de la transmisión.
    if (srcProtocol == "arp") {
        offset += HT_Hp->updateGet1D2DStats(srcMAC, dstMAC, timestamp, datagramSize, result + offset);
    } else {
        offset += HT_Hp->updateGet1D2DStats(srcIP + srcProtocol, dstIP + dstProtocol, timestamp,
                                            datagramSize, result + offset);
    }
    return offset;
}

