#include "termografiamanager.h"
#include <QImage>
#include <QColor>

TermografiaManager::TermografiaManager(QObject *parent)
    : QObject(parent), m_termogramaActivo(nullptr)
{
}

Termograma* TermografiaManager::obtenerTermogramaCompleto(long id) {
    // Simulación de carga desde DAO
    static Termograma t;
    t = m_termogramaDAO.getById(id);

    // Si la matriz no está cargada, la cargamos ahora
    if (t.matrizDatos.isEmpty() && !t.rutaFisica.isEmpty()) {
        t.matrizDatos = m_ingestionEngine.parsearMatrizTermica(t.rutaFisica, t.ancho, t.alto);
    }

    m_termogramaActivo = &t;
    return m_termogramaActivo;
}

// Método simple para que la UI obtenga el puntero sin buscar en BD
Termograma* TermografiaManager::obtenerTermogramaActivo() {
    return m_termogramaActivo;
}


QImage TermografiaManager::generarImagenVisual(long idTermograma, float minRango, float maxRango) {
    Q_UNUSED(idTermograma); // Limpia el warning de "unused parameter"

    if (!m_termogramaActivo) return QImage();

    int w = m_termogramaActivo->ancho;
    int h = m_termogramaActivo->alto;

    if (w <= 0 || h <= 0) return QImage();

    QImage img(w, h, QImage::Format_RGB32);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float temp = m_termogramaActivo->getTemperatura(x, y);

            // Normalizar entre 0 y 1 según el rango de la leyenda (20-80°)
            float norma = (temp - minRango) / (maxRango - minRango);
            norma = qBound(0.0f, norma, 1.0f);

            // Rampa de color (Azul frío -> Rojo caliente)
            int r = static_cast<int>(norma * 255);
            int b = static_cast<int>((1.0f - norma) * 255);
            img.setPixel(x, y, qRgb(r, 0, b));
        }
    }
    return img;
}

// Implementación de carga de lote
// Actualiza la carga para llenar la lista interna
IngestionEngine::ResumenCarga TermografiaManager::cargarLoteDesdeJSON(const QString& rutaJson) {
    IngestionEngine::ResumenCarga resumen = {0, 0};

    // 1. Limpiamos la lista previa
    m_listaTermogramas = m_ingestionEngine.procesarManifiesto(rutaJson);

    // 2. Guardamos en el DAO (opcional, según tu lógica de BD)
    for(const Termograma& t : m_listaTermogramas) {
        if(m_termogramaDAO.insert(t)) {
            resumen.totalCargados++;
        } else {
            resumen.errores++;
        }
    }

    // 3. Activamos el primero de la lista si existe
    if(!m_listaTermogramas.isEmpty()) {
        m_indiceActivo = 0;
        m_termogramaActivo = &m_listaTermogramas[0];
    }

    return resumen;
}

// Ahora este método sí encontrará las variables
Termograma* TermografiaManager::obtenerTermogramaActivo(int index) {
    if (index >= 0 && index < m_listaTermogramas.size()) {
        m_indiceActivo = index;
        m_termogramaActivo = &m_listaTermogramas[index];
        return m_termogramaActivo;
    }
    return nullptr;
}
