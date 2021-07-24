//
// Created by Yang Bo on 2020/5/9.
//

#ifndef KITSUNE_CPP_FEATUREEXTRACTOR_H
#define KITSUNE_CPP_FEATUREEXTRACTOR_H

#include <cstdio>
#include "utils.h"
#include "netStat.h"

/**
 *  Clase de extracción de características
 *  Actualmente puede hacer:
 *  1. Extraiga características de pcap y obtenga vectores de instancia
 *  2. Leer características de los archivos tsv, csv del paquete y obtener vectores de instancia
 *  3. Leer el vector de instancia directamente desde el archivo tsv, csv del vector de instancia
 *  Qué hacer a continuación:
 *  4. Captura de paquetes en línea, obtenga directamente el vector de instancia de cada paquete
 */


// Tipo enumerado, tipo de archivo definido
enum FileType {
    PCAP, PacketTSV, PacketCSV, FeatureCSV, FeatureTSV, OnlineNetDevice
};

// Responsable de obtener vectores de características. Se puede leer desde archivos pcap, tsv o una línea de vectores se puede leer directamente desde tipos de archivos como FeatureCSV
class FE {
private:
    TsvReader *tsvReader = nullptr;
    NetStat *netStat = nullptr;     // = nullptr
    FileType fileType; // Tipo de archivo actual
public:
    // netStat usa el constructor de ventana de tiempo predeterminado y lee el archivo de características del paquete tsv de manera predeterminada
    FE(const char *filename, FileType ft = PacketTSV);

    // El constructor de la ventana de tiempo especificada, el archivo de características del paquete tsv leído por defecto
    FE(const char *filename, const std::vector<double> &lambdas, FileType ft = PacketTSV);

    // Incinerador de basuras
    ~FE() {
        delete tsvReader;
        if (netStat != nullptr)delete netStat;
    }

    // Obtenga el siguiente vector de instancia y guárdelo como resultado
    int nextVector(double *result);

    // Devuelve el tamaño del vector de instancia generado cada vez.
    inline int getVectorSize() { return netStat->getVectorSize(); }

};



#endif //KITSUNE_CPP_FEATUREEXTRACTOR_H
