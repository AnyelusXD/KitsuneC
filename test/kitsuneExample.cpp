//
// Created by Yang Bo on 2020/5/15.
//

#include "../include/featureExtractor.h"
#include "../include/neuralnet.h"
#include "../include/kitNET.h"
#include "test.h"

// Ejemplo de prueba simple de Kitsune

void kitsuneExample() {
    const char *filename = "F:\\Dataset\\KITSUNE\\Mirai\\Mirai_pcap.pcap.tsv";
    const int FM_train_num = 5000; // La cantidad de mapas de características de entrenamiento requeridos
    const int AD_train_num = 50000; // El número de módulos de detección de anomalías de formación necesarios
    const int KitNET_train_num = AD_train_num + FM_train_num;  // El número necesario para entrenar KitNET
    const int max_AE = 10; // La mayor escala de codificador automático

    auto fe = new FE(filename, PacketTSV);  // Inicializar el módulo de extracción de características

    int sz = fe->getVectorSize(); // Obtenga el número requerido para la extracción de características

    auto kitNET = new KitNET(sz, max_AE, FM_train_num); // Inicializar el módulo kitNET

    auto *x = new double[sz]; // Inicializar el búfer almacenando el vector de características de entrada

    FILE *fp = fopen("RMSE.txt", "w");
    int now_packet = 0;
    while (fe->nextVector(x)) {
        ++now_packet;
        if (now_packet <= KitNET_train_num)
            fprintf(fp, "%.15f\n", kitNET->train(x));
        else
            fprintf(fp, "%.15f\n", kitNET->execute(x));

        if (now_packet % 1000 == 0)printf("%d\n", now_packet);
    }

    printf("total packets is %d\n", now_packet);
    fclose(fp);
    delete[] x;
    delete fe;
    delete kitNET;
}