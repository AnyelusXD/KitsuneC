//
// Created by Yang Bo on 2020/5/9.
//

#ifndef KITSUNE_CPP_NETSTAT_H
#define KITSUNE_CPP_NETSTAT_H


/**
 *  Característica del algoritmo estadístico de Kitsune implementado
 *  Este archivo contiene clases: NetStat, IncStat, IncStatCov, IncStatDB
 *  También hay dos clases de herramientas: QueueFixed (cola de tamaño fijo), Extractor (cálculo de extrapolación lagrangiana)
 */

#include <vector>
#include <string>
#include <map>
#include <cmath>


/**
 *  一个Una cola de tamaño fijo, solo inserción circular pero no eliminación
 */
class QueueFixed {
public:
    // Capacidad de la cola, elementos estáticos
    static const int QueueCapacity = 3;

private:
    double array[QueueFixed::QueueCapacity];
    // El índice de la ubicación de inserción actual
    int now_index;
    // Número actual de elementos
    int now_size;

public:
    // Constructor
    QueueFixed() {
        now_index = now_size = 0;
    }

    // Insertar nuevo elemento
    void insert(double x);

    // Expanda la cola en una matriz, cópiela en arr y devuelva el número de elementos de la matriz
    int unroll(double *arr);

    // Devuelve el último elemento que entró en la cola.
    double getLast();

};

class Extrapolator {
private:
    // Mantener dos colas, una de las cuales es la variable independiente t, y la otra es el valor de la función v
    QueueFixed tQ, vQ;
    // Guardar el valor expandido
    double tArr[QueueFixed::QueueCapacity];
    double vArr[QueueFixed::QueueCapacity];

public:
    // Insertar nuevo elemento
    void insert(double t, double v) {
        tQ.insert(t);
        vQ.insert(v);
    }

    // Utilice la interpolación lagrangiana para predecir el próximo valor
    double predict(double t);
};


class IncStatCov; // Debido a la referencia cruzada, declare esta clase primero


/**
 *  IncStat Estadísticas de datos incrementales para un flujo específico
 */
class IncStat {
private:
    // El factor de atenuación de la transmisión, que es el puntero de la lista de la ventana de tiempo.
    std::vector<double> *lambdas;

    // Lista de sumas lineales, sumas de cuadrados y pesos de estadísticas, donde el i - ésimo valor corresponde a la información estadística de la i - ésima ventana de tiempo
    double *CF1 = nullptr, *CF2 = nullptr, *w = nullptr;

    // Última marca de tiempo
    double lastTimestamp;

    // ¿Son válidas la media, la varianza y la desviación estándar actuales (es necesario volver a calcular el falso)?
    bool mean_valid, var_valid, std_valid;

    // ¿Es el tipo diferente ? Si es cierto, use la nueva marca de tiempo como datos(también para calcular las estadísticas de la marca de tiempo)
    bool isTypeDiff;

public:
    // ID del índice de transmisión actual
    std::string ID;

    // La lista actual de media, varianza y desviación estándar, el i-ésimo valor corresponde a la información estadística de la i-ésima ventana de tiempo
    double *cur_mean = nullptr, *cur_var = nullptr, *cur_std = nullptr;

    // La colección de transmisiones conectadas a la transmisión actual
    std::vector<IncStatCov *> covs;

    // Constructor, los parámetros son el ID de flujo actual, lambda, marca de tiempo de inicialización, si se debe usar la marca de tiempo como información estadística
    IncStat(const std::string &_ID, std::vector<double> *_lambdas, double init_time = 0, bool isTypediff = false);

    // Destructor, para destruir el contenido del puntero a la información lateral.
    ~IncStat();

    // Una función para insertar nuevos datos, los parámetros son v estadísticas, t marca de tiempo
    void insert(double v, double t = 0);

    // Realice la atenuación, el parámetro es la marca de tiempo actual
    void processDecay(double timestamp);

    // Calcule la media
    void calMean();

    // Calcular la varianza
    void calVar();

    // Calcule la desviación estándar
    void calStd();

    // Obtenga toda la información estadística unidimensional (peso, media, varianza) y agregue el resultado al resultado, devuelva el número de datos agregados
    int getAll1DStats(double *result);
};


/**
 * IncStatCov 
Mantener la relación entre las dos corrientes (bordes conectados),
 * 
Almacena los punteros de los dos flujos y la información estadística entre ellos.
 */
class IncStatCov {
private:
    // Puntero a la lista de ventanas de tiempo mantenidas
    std::vector<double> *lambdas;
    // La suma de productos menos la media de cada valor , sum (A-uA)(B-uB), Parte del numerador de la covarianza
    double *CF3 = nullptr;
    // Peso actual
    double *w3 = nullptr;
    // Última marca de tiempo
    double lastTimestamp;
    // Dos clases de extrapolación lagrangiana
    Extrapolator ex1, ex2;

public:
    // Dos punteros de flujo :
    IncStat *incS1, *incS2;
    // El número de referencias, si es 0, se destruirá
    int refNum;

    // Constructor, los parámetros son punteros a dos flujos, punteros lambdas y marca de tiempo inicial
    IncStatCov(IncStat *inc1, IncStat *inc2, std::vector<double> *l, double init_time) {
        lambdas = l;
        incS1 = inc1;
        incS2 = inc2;
        lastTimestamp = init_time;

        CF3 = new double[lambdas->size()];
        for (size_t i = 0; i < lambdas->size(); ++i)CF3[i] = 0;
        w3 = new double[lambdas->size()];
        // Evitar la división por 0
        for (size_t i = 0; i < lambdas->size(); ++i)w3[i] = 1e-20;
    }

    // Destructor, (la instancia de la clase a la que apunta se llamará cuando se destruya el mapa), solo necesita eliminar el nuevo
    ~IncStatCov() {
        if (CF3 != nullptr) delete[] CF3;
        if (w3 != nullptr) delete[] w3;
    }

    //Actualice estadísticas como la covarianza de estos dos flujos.
    //Solo se puede llamar después de que se actualice una de las dos secuencias y, a continuación, el parámetro es el ID de la secuencia actualizada y la v y la t utilizada para la actualización de la secuencia actualizada.
    //Es decir, después de actualizar uno de los métodos de inserción de flujo, este método se llama inmediatamente para actualizar las estadísticas relevantes.
    void updateCov(const std::string &ID, double v, double t);

    // Realizar una función de decaimiento
    void processDecay(double t);

    // Calcule el radio (raíz cuadrada de la suma de la varianza) de las dos corrientes y devuelva el número de datos agregados
    int getRadius(double *result);

    // Calcule la raíz cuadrada de la suma de los cuadrados de las dos corrientes y devuelva el número de datos agregados
    int getMagnitude(double *result);

    // Calcule la covarianza de las dos corrientes y devuelva el número de datos agregados
    int getCov(double *result);

    // Calcule el coeficiente de correlación de las dos corrientes y devuelva el número de datos agregados
    int getPcc(double *result);

    //Obtenga toda la información estadística bidimensional [radio, magnitud, cov, pcc], devuelva el número de datos agregados
    int getAll2DStats(double *result);
};


/**
 *  IncStatDB Mantener una colección de estadísticas actuales.
 */
class IncStatDB {
private:
    // Una colección de flujos estadísticos, la cadena es el valor clave correspondiente y el valor es un puntero al flujo correspondiente.
    std::map<std::string, IncStat *> stats;
    // lambdas Puntero a la lista de ventanas de tiempo mantenidas
    std::vector<double> *lambdas;

public:
    // Constructor, pasa la lista de punteros de la ventana de tiempo
    IncStatDB(std::vector<double> *l) {
        lambdas = l;
    }

    // Actualice la información unidimensional del flujo especificado, agregue el valor estadístico[peso, media, estándar] al resultado y devuelva el número de datos agregados
    int updateGet1DStats(const std::string &ID, double t, double v, double *result,
                         bool isTypeDiff = false);

    // Actualice la información bidimensional de la transmisión especificada y agregue [radio, magnitud, cov, pcc] al resultado
    // Los parámetros son : el ID del primer flujo, el ID del segundo flujo, las estadísticas del primer flujo, la marca de tiempo, el puntero a la matriz de resultados y la cantidad de datos agregados
    int updateGet2DStats(const std::string &ID1, const std::string &ID2, double t1, double v1,
                         double *result, bool isTypediff = false);

    // Actualiza la información unidimensional y bidimensional de la transmisión especificada y devuelve un[peso, media, estándar] y bidimensional[radio, magnitud, cov, pcc].
    // Los parámetros son : el ID de la transmisión, la marca de tiempo, las estadísticas, el puntero a la matriz de resultados y el último si se establece en verdadero, la marca de tiempo se usa como estadísticas
    // Devuelve el número de datos agregados
    int updateGet1D2DStats(const std::string &ID1, const std::string &ID2, double t1,
                           double v1, double *result, bool isTypediff = false) {
        int offset = updateGet1DStats(ID1, t1, v1, result, isTypediff);
        return offset + updateGet2DStats(ID1, ID2, t1, v1, result + offset, isTypediff);
    }

    // El destructor, que libera todos los valores apuntados por el conjunto de punteros en el incStat mantenido
    ~IncStatDB() {
//        std::fprintf(stderr, "the number of incStat is: %d\n", stats.size());
//        int ans = 0;
//        int m = 0;
        for (auto it:stats) {
//            ans += it.second->covs.size();
//            if (it.second->covs.size() > m)m = it.second->covs.size();
            delete it.second;
        }
//        std::fprintf(stderr, "the number of incStatCov is %d\n", ans);
//        std::fprintf(stderr, "the max number of incStatCov is %d\n", m);
    }
};

/**
 * NetStat La clase mantiene estadísticas de red actuales, incluidas estadísticas sobre hosts, fluctuación de paquetes, canales de red, etc.

 * El vector de instancia responsable de generar estadísticas a partir del paquete.
 */

class NetStat {
private:
    // Ventana de tiempo
    std::vector<double> lambdas;
    // Estadísticas de cuatro tipos de corrientes,
    //1. HT_jit: Estadísticas de fluctuación entre host y host, solo 1 dimensión (3 características)
    //2. HT_MI: Estadísticas sobre la relación entre el flujo de envío MAC-IP, solo 1 dimensión (3 características)
    //3. HT_H: mantiene estadísticas de ancho de banda unidimensionales del flujo de envío del host de origen y estadísticas bidimensionales del flujo de envío del host de origen
    //4. HT_Hp: mantiene las estadísticas de ancho de banda unidimensionales del flujo de envío del puerto del host de origen y las estadísticas bidimensionales del flujo de envío del puerto del host de origen (7 funciones). Esto es diferente del HT_H anterior en que el valor clave es ip + puerto , considerando cada puerto
    IncStatDB *HT_jit = nullptr, *HT_MI = nullptr, *HT_H = nullptr, *HT_Hp = nullptr;

public:
    // Constructor, los parámetros son lambdas
    NetStat(const std::vector<double> &l);

    // Sin constructor de parámetros, use lambdas predeterminadas
    NetStat();

    // La función de llamada principal, pasa la información de un paquete y devuelve el vector estadístico correspondiente
    // Los parámetros son : MAC de origen, MCA de destino, IP de origen, tipo de protocolo IP, IP de destino, tipo de protocolo IP de destino, tamaño del paquete, marca de tiempo del paquete
    int updateAndGetStats(const std::string &srcMAC, const std::string &dstMAC,
                          const std::string &srcIP, const std::string &srcProtocol,
                          const std::string &dstIP, const std::string &dstProtocol,
                          double datagramSize, double timestamp, double *result);

    // Devuelve la dimensión del vector de instancia estadístico generado, actualmente cada lambda corresponde a 20 características
    int getVectorSize() { return lambdas.size() * 20; }

    // Destructor, elimine cuatro instancias de nuevo
    ~NetStat() {
        delete HT_H;
        delete HT_Hp;
        delete HT_MI;
        delete HT_jit;
    }
};



#endif //KITSUNE_CPP_NETSTAT_H
