#include "termografiamanager.h"
#include <QImage>
#include <QColor>
#include <QDebug>

TermografiaManager::TermografiaManager(QObject *parent)
    : QObject(parent)
    , m_termogramaActivo(nullptr)
    , m_indiceActivo(-1)
{
    qDebug() << "TermografiaManager inicializado";
}

void TermografiaManager::setIndiceActivo(int index) {
    if (index >= 0 && index < m_listaTermogramas.size()) {
        m_indiceActivo = index;
        m_termogramaActivo = &m_listaTermogramas[index];
        qDebug() << "Termograma activo cambiado a índice:" << index;
    } else {
        qDebug() << "Advertencia: Índice fuera de rango:" << index;
    }
}

Termograma* TermografiaManager::obtenerTermogramaActivo() {
    return m_termogramaActivo;
}

Termograma* TermografiaManager::obtenerTermogramaActivo(int index) {
    if (index >= 0 && index < m_listaTermogramas.size()) {
        m_indiceActivo = index;
        m_termogramaActivo = &m_listaTermogramas[index];
        return m_termogramaActivo;
    }

    qDebug() << "Error: Índice no válido:" << index;
    return nullptr;
}

Termograma* TermografiaManager::obtenerTermogramaCompleto(long id) {
    // Cargar desde base de datos
    Termograma t = m_termogramaDAO.getById(id);

    // Si la matriz está vacía, cargar desde archivo
    if (t.matrizDatos.isEmpty() && !t.rutaFisica.isEmpty()) {
        t.matrizDatos = m_ingestionEngine.parsearMatrizTermica(t.rutaFisica, t.ancho, t.alto);
    }

    // Actualizar termograma activo (temporal)
    static Termograma termogramaTemp;
    termogramaTemp = t;
    m_termogramaActivo = &termogramaTemp;

    return m_termogramaActivo;
}

IngestionEngine::ResumenCarga TermografiaManager::cargarLoteDesdeJSON(const QString& rutaJson) {
    IngestionEngine::ResumenCarga resumen = {0, 0};

    qDebug() << "Iniciando carga de dataset desde:" << rutaJson;

    // 1. Procesar el archivo JSON
    m_listaTermogramas = m_ingestionEngine.procesarManifiesto(rutaJson);

    if (m_listaTermogramas.isEmpty()) {
        qDebug() << "Error: No se cargaron termogramas";
        return resumen;
    }

    // 2. Persistir en base de datos
    for (const Termograma& t : m_listaTermogramas) {
        if (m_termogramaDAO.insert(t)) {
            resumen.totalCargados++;
        } else {
            resumen.errores++;
        }
    }

    // 3. Seleccionar automáticamente el primer elemento
    if (!m_listaTermogramas.isEmpty()) {
        setIndiceActivo(0);
    }

    qDebug() << "Resumen de carga - Exitosos:" << resumen.totalCargados
             << "| Errores:" << resumen.errores;

    return resumen;
}

QImage TermografiaManager::generarImagenVisual(long idTermograma, float minRango, float maxRango) {
    Q_UNUSED(idTermograma);

    if (!m_termogramaActivo || m_termogramaActivo->matrizDatos.isEmpty()) {
        qDebug() << "Error: No hay termograma activo o matriz vacía";
        return QImage();
    }

    int w = m_termogramaActivo->ancho;
    int h = m_termogramaActivo->alto;

    if (w <= 0 || h <= 0) {
        qDebug() << "Error: Dimensiones inválidas:" << w << "x" << h;
        return QImage();
    }

    // MEJORA: Crear imagen de alta resolución (4x)
    int factorUpscale = 4;
    int wHD = w * factorUpscale;
    int hHD = h * factorUpscale;

    QImage imgHD(wHD, hHD, QImage::Format_ARGB32);
    imgHD.fill(Qt::transparent);

    // Generar mapa térmico con interpolación bicúbica
    for (int yHD = 0; yHD < hHD; ++yHD) {
        for (int xHD = 0; xHD < wHD; ++xHD) {
            // Mapear coordenadas HD a coordenadas originales
            float xOrig = static_cast<float>(xHD) / factorUpscale;
            float yOrig = static_cast<float>(yHD) / factorUpscale;

            // Interpolación bilinear para suavidad
            int x0 = static_cast<int>(xOrig);
            int y0 = static_cast<int>(yOrig);
            int x1 = qMin(x0 + 1, w - 1);
            int y1 = qMin(y0 + 1, h - 1);

            float dx = xOrig - x0;
            float dy = yOrig - y0;

            // Obtener temperaturas de los 4 píxeles cercanos
            float t00 = m_termogramaActivo->getTemperatura(x0, y0);
            float t10 = m_termogramaActivo->getTemperatura(x1, y0);
            float t01 = m_termogramaActivo->getTemperatura(x0, y1);
            float t11 = m_termogramaActivo->getTemperatura(x1, y1);

            // Interpolación bilinear
            float temp = t00 * (1 - dx) * (1 - dy) +
                        t10 * dx * (1 - dy) +
                        t01 * (1 - dx) * dy +
                        t11 * dx * dy;

            // Normalizar temperatura
            float norma = (temp - minRango) / (maxRango - minRango);
            norma = qBound(0.0f, norma, 1.0f);

            // Paleta térmica mejorada (más contrastada)
            int r, g, b;

            if (norma < 0.20f) {
                // Azul oscuro a Azul
                float t = norma / 0.20f;
                r = 0;
                g = static_cast<int>(t * 100);
                b = static_cast<int>(150 + t * 105);
            } else if (norma < 0.40f) {
                // Azul a Cian
                float t = (norma - 0.20f) / 0.20f;
                r = 0;
                g = static_cast<int>(100 + t * 155);
                b = 255;
            } else if (norma < 0.60f) {
                // Cian a Verde
                float t = (norma - 0.40f) / 0.20f;
                r = 0;
                g = 255;
                b = static_cast<int>(255 * (1.0f - t));
            } else if (norma < 0.80f) {
                // Verde a Amarillo
                float t = (norma - 0.60f) / 0.20f;
                r = static_cast<int>(t * 255);
                g = 255;
                b = 0;
            } else {
                // Amarillo a Rojo brillante
                float t = (norma - 0.80f) / 0.20f;
                r = 255;
                g = static_cast<int>(255 * (1.0f - t));
                b = 0;
            }

            imgHD.setPixel(xHD, yHD, qRgba(r, g, b, 255));
        }
    }

    // Aplicar suavizado antialiasing para eliminar pixelado
    QImage imgFinal = imgHD.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    return imgFinal;
}

void TermografiaManager::guardarPuntoCaliente(PuntoCaliente p, long idTermograma) {
    if (m_puntoCalienteDAO.save(p, idTermograma)) {
        qDebug() << "Punto caliente guardado exitosamente";
    } else {
        qDebug() << "Error al guardar punto caliente";
    }
}

void TermografiaManager::exportarReporte(int formato, long idTermograma) {
    Q_UNUSED(formato);
    Q_UNUSED(idTermograma);

    qDebug() << "Exportación de reporte no implementada aún";
}
