#ifndef TERMOGRAFIAMANAGER_H
#define TERMOGRAFIAMANAGER_H

#include <QObject>
#include <QImage>
#include <QList>
#include "entities.h"
#include "engines.h"
#include "daos.h"
#include "Core_global.h"

class CORE_EXPORT TermografiaManager : public QObject {
    Q_OBJECT

public:
    explicit TermografiaManager(QObject *parent = nullptr);

    // Getters y Setters de estado
    void setIndiceActivo(int index);
    Termograma* obtenerTermogramaActivo();
    Termograma* obtenerTermogramaActivo(int index);
    Termograma* obtenerTermogramaCompleto(long id);

    // Lógica de Negocio
    IngestionEngine::ResumenCarga cargarLoteDesdeJSON(const QString& rutaJson);
    void guardarPuntoCaliente(PuntoCaliente punto, long idTermograma);
    void exportarReporte(int formatoEnum, long idCampaña);

    // UI Helpers
    QImage generarImagenVisual(long idTermograma, float minRango, float maxRango);

private:
    IngestionEngine m_ingestionEngine;
    AnalysisEngine m_analysisEngine;
    TermogramaDAO m_termogramaDAO;
    PuntoCalienteDAO m_puntoCalienteDAO;
    ElementoRedDAO m_elementoRedDAO;

    QList<Termograma> m_listaTermogramas;
    Termograma* m_termogramaActivo = nullptr;
    int m_indiceActivo = -1;
};

#endif
