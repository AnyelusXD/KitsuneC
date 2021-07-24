//
// Created by Yang Bo on 2020/5/9.
//

#include "../include/featureExtractor.h"


// netStat Utilice el constructor de la ventana de tiempo predeterminada y lea el archivo de características del paquete de tsv de forma predeterminada
FE::FE(const char *filename, FileType ft) {
    fileType = ft;
    netStat = new NetStat();
    if (fileType == PacketTSV || fileType == FeatureTSV) {// El delimitador es una pestaña
        tsvReader = new TsvReader(filename, '\t');
    } else if (fileType == PacketCSV || fileType == FeatureCSV) {// El separador es ','
        tsvReader = new TsvReader(filename, ',');
    } else if (fileType == PCAP) { // Necesita ser convertido a un archivo tsv
        tsvReader = new TsvReader(pcap2tcv(filename));
    }
    // Para leer el archivo de características del paquete, debe utilizar las estadísticas de netStat
    if (fileType == PCAP || fileType == PacketCSV || fileType == PacketTSV) {
        // Hay un encabezado, por lo que primero debe leer el encabezado
        tsvReader->nextLine();
    }
}

// El constructor de la ventana de tiempo especificada, el archivo de características del paquete tsv leído por defecto
FE::FE(const char *filename, const std::vector<double> &lambdas, FileType ft) {
    fileType = ft;
    netStat = new NetStat(lambdas);
    if (fileType == PacketTSV || fileType == FeatureTSV) {// El delimitador es una pestaña
        tsvReader = new TsvReader(filename, '\t');
    } else if (fileType == PacketCSV || fileType == FeatureCSV) {// El separador es ','
        tsvReader = new TsvReader(filename, ',');
    } else if (fileType == PCAP) { // Necesita ser convertido a un archivo tsv
        tsvReader = new TsvReader(pcap2tcv(filename));
    }
    // Para leer el archivo de características del paquete, debe utilizar las estadísticas de netStat
    if (fileType == PCAP || fileType == PacketCSV || fileType == PacketTSV) {
        // Hay un encabezado, por lo que primero debe leer el encabezado
        tsvReader->nextLine();
    }
}


// Lea las características de una fila de paquetes del lector y páselos a netstat para obtener el vector del siguiente conjunto de instancias.
// Si tiene éxito, devuelve el número de vectores; de lo contrario, devuelve 0
int FE::nextVector(double *result) {
    int cols = tsvReader->nextLine();
    if (cols == 0)return 0;
    if (fileType == FeatureTSV || fileType == FeatureCSV) { // Si lee la información del vector directamente, lea el doble directamente
        int num = getVectorSize();
        if (cols < num)return 0;
        for (int i = 0; i < num; ++i)result[i] = tsvReader->getDouble(i);
        return num;
    } else { // Estadísticas incrementales con netStat
        std::string srcIP, dstIP;
        if (tsvReader->hasValue(4)) {// Ipv4
            srcIP = tsvReader->getString(4);
            dstIP = tsvReader->getString(5);
        } else { // Ipv6
            srcIP = tsvReader->getString(17);
            dstIP = tsvReader->getString(18);
        }
        std::string srcport, dstport;
        if (tsvReader->hasValue(6)) {//tcp
            srcport = tsvReader->getString(6);
            dstport = tsvReader->getString(7);
        } else if (tsvReader->hasValue(8)) { // udp
            srcport = tsvReader->getString(8);
            dstport = tsvReader->getString(9);
        } else { // No es tcp ni udp, puede ser un paquete de 1,2 capas como arp o icmp
            if (tsvReader->hasValue(10)) { // icmp
                srcport = dstport = "icmp";
            } else if (tsvReader->hasValue(12)) { // arp
                srcport = dstport = "arp";
                // Utilice la ip de origen y la ip de destino en el paquete arp como información de ip
                srcIP = tsvReader->getString(14);
                dstIP = tsvReader->getString(16);
            } else { // Otros protocolos, utilizan la asignación de MAC de origen y destino
                srcIP = tsvReader->getString(2);
                dstIP = tsvReader->getString(3);
            }
        }
        return netStat->updateAndGetStats(tsvReader->getString(2), tsvReader->getString(3),
                                          srcIP, srcport, dstIP, dstport, tsvReader->getDouble(1),
                                          tsvReader->getDouble(0), result);
    }
}
