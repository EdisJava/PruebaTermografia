#include "engines.h"

AnalysisEngine::AnalysisEngine() {

}

QList<PuntoCaliente> AnalysisEngine::detectarPuntosCalientesAuto(const Termograma& t, float umbral) {
    QList<PuntoCaliente> encontrados;
    if (t.matrizDatos.isEmpty()) return encontrados;

    for (int i = 0; i < t.matrizDatos.size(); ++i) {
        if (t.matrizDatos[i] >= umbral) {
            PuntoCaliente p;
            p.temperaturaMax = t.matrizDatos[i];
            p.pixelX = i % t.ancho;
            p.pixelY = i / t.ancho;
            p.tipo = TipoPunto::SUGERIDO;
            p.validado = false;
            encontrados.append(p);

            if (encontrados.size() > 50) break;
        }
    }
    return encontrados;
}

float AnalysisEngine::validarPunto(const Termograma& t, int x, int y) {
    return t.getTemperatura(x, y);
}
