#ifndef ENGINES_H
#define ENGINES_H

#include "entities.h"
#include "Core_global.h"
#include <QFile>
#include <QVector>

class CORE_EXPORT AnalysisEngine {
public:
    AnalysisEngine();
    QList<PuntoCaliente> detectarPuntosCalientesAuto(const Termograma& t, float umbral);
    float validarPunto(const Termograma& t, int x, int y);
};

class CORE_EXPORT IngestionEngine {
public:
    struct ResumenCarga {
        int totalCargados;
        int errores;
    };

    IngestionEngine();
    QList<Termograma> procesarManifiesto(const QString& jsonFile);
    QVector<float> parsearMatrizTermica(const QString& rutaCSV, int& w, int& h);
};

#endif
