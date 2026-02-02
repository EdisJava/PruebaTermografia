#include "mainwindow.h"
#include <QApplication>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <random>
#include <vector>
#include <cmath>
#include <fstream>

/**
 * @brief Genera archivos CSV de prueba con datos térmicos realistas.
 * Simula patrones térmicos con puntos calientes gaussianos distribuidos aleatoriamente.
 */
void generarDatosPrueba(const std::string& nombre, int w, int h) {
    std::ofstream file(nombre);
    if (!file.is_open()) {
        qDebug() << "Error: No se pudo crear archivo:" << QString::fromStdString(nombre);
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    // Distribuciones para generar datos realistas
    std::uniform_real_distribution<float> distTempAmbiente(18.0f, 24.0f);
    std::uniform_int_distribution<int> distNumSpots(3, 7);
    std::uniform_int_distribution<int> distPosX(10, w - 10);
    std::uniform_int_distribution<int> distPosY(10, h - 10);
    std::uniform_real_distribution<float> distIntensidad(40.0f, 90.0f);
    std::uniform_real_distribution<float> distRadio(5.0f, 15.0f);
    std::normal_distribution<float> distRuido(0.0f, 0.8f);

    // Temperatura base
    float tempBase = distTempAmbiente(gen);

    // Estructura para puntos calientes
    struct HotSpot {
        int x, y;
        float maxTemp;
        float sigma;
    };

    // Generar varios puntos calientes
    int cantidadSpots = distNumSpots(gen);
    std::vector<HotSpot> spots;

    for (int i = 0; i < cantidadSpots; ++i) {
        spots.push_back({
            distPosX(gen),
            distPosY(gen),
            distIntensidad(gen),
            distRadio(gen)
        });
    }

    // Generar matriz de temperaturas
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            // Gradiente vertical suave (más calor arriba)
            float gradienteY = (static_cast<float>(h - y) / h) * 4.0f;

            // Ruido aleatorio
            float ruido = distRuido(gen);

            // Temperatura inicial
            float tempPixel = tempBase + gradienteY + ruido;

            // Añadir contribución de puntos calientes (distribución gaussiana)
            for (const auto& spot : spots) {
                float dx = x - spot.x;
                float dy = y - spot.y;
                float distSq = dx * dx + dy * dy;

                // Fórmula gaussiana
                float aporte = spot.maxTemp * std::exp(-distSq / (2.0f * std::pow(spot.sigma, 2.0f)));

                if (aporte > 0.1f) {
                    tempPixel += aporte;
                }
            }

            // Escribir valor (con coma como separador)
            file << std::fixed << std::setprecision(2) << tempPixel;

            if (x < w - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
    qDebug() << "✓ Archivo generado:" << QString::fromStdString(nombre)
             << "(" << w << "x" << h << ")";
}

/**
 * @brief Genera un archivo JSON manifiesto para las pruebas
 */
void generarManifiestoJSON() {
    QJsonObject root;

    // Array de apoyos
    QJsonArray apoyos;
    for (int i = 1; i <= 3; ++i) {
        QJsonObject apoyo;
        apoyo["nombre"] = QString::number(i);

        QJsonObject termograma;
        termograma["url"] = QString("data_640x480_%1.csv").arg(i);

        QJsonObject coordenada;
        coordenada["x"] = 40.4168 + (i * 0.001);
        coordenada["y"] = -3.7038 + (i * 0.001);
        coordenada["z"] = 650.0 + (i * 10);

        termograma["coordenada"] = coordenada;
        apoyo["termograma"] = termograma;

        apoyos.append(apoyo);
    }
    root["Apoyo"] = apoyos;

    // Guardar JSON
    QJsonDocument doc(root);
    QFile file("dataset_prueba.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "✓ Manifiesto JSON generado: dataset_prueba.json";
    } else {
        qDebug() << "✗ Error al crear manifiesto JSON";
    }
}

/**
 * @brief Inicializa la base de datos SQLite.
 */
bool inicializarBaseDeDatos() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("dalia_termografia.db");

    if (!db.open()) {
        qDebug() << "✗ Error SQL:" << db.lastError().text();
        return false;
    }

    QSqlQuery q;

    // Tabla de termogramas
    q.exec("CREATE TABLE IF NOT EXISTS termogramas ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "nombre_archivo TEXT, "
           "ruta_fisica TEXT, "
           "fecha DATETIME, "
           "x REAL, "
           "y REAL, "
           "z REAL, "
           "ancho INTEGER, "
           "alto INTEGER)");

    // Tabla de puntos calientes
    q.exec("CREATE TABLE IF NOT EXISTS puntos_calientes ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "id_termograma INTEGER, "
           "temp_max REAL, "
           "x INTEGER, "
           "y INTEGER, "
           "tipo INTEGER, "
           "comentario TEXT, "
           "validado BOOLEAN DEFAULT 0, "
           "FOREIGN KEY(id_termograma) REFERENCES termogramas(id))");

    // Tabla de elementos de red
    q.exec("CREATE TABLE IF NOT EXISTS elementos_red ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "codigo TEXT UNIQUE, "
           "tipo INTEGER, "
           "latitud REAL, "
           "longitud REAL)");

    // Tabla de relación
    q.exec("CREATE TABLE IF NOT EXISTS rel_elemento_termograma ("
           "id_elemento INTEGER, "
           "id_termograma INTEGER, "
           "PRIMARY KEY(id_elemento, id_termograma), "
           "FOREIGN KEY(id_elemento) REFERENCES elementos_red(id), "
           "FOREIGN KEY(id_termograma) REFERENCES termogramas(id))");

    qDebug() << "✓ Base de datos inicializada correctamente";
    return true;
}

/**
 * @brief Función principal
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("DALIA");
    a.setApplicationVersion("1.0");
    a.setOrganizationName("ThermalAnalysis");

    qDebug() << "=================================================";
    qDebug() << "  DALIA - Sistema de Análisis Termográfico";
    qDebug() << "=================================================";

    // 1. Generar datos de prueba de ALTA RESOLUCIÓN
    qDebug() << "\n[1/3] Generando datos de prueba de alta resolución...";
    generarDatosPrueba("data_640x480_1.csv", 640, 480);
    generarDatosPrueba("data_640x480_2.csv", 640, 480);
    generarDatosPrueba("data_640x480_3.csv", 640, 480);
    generarManifiestoJSON();

    // 2. Inicializar base de datos
    qDebug() << "\n[2/3] Inicializando base de datos...";
    if (!inicializarBaseDeDatos()) {
        qDebug() << "✗ Error crítico: No se pudo inicializar la base de datos";
        return -1;
    }

    // 3. Lanzar aplicación
    qDebug() << "\n[3/3] Iniciando interfaz gráfica...";
    MainWindow w;
    w.setWindowTitle("DALIA - Análisis Termográfico Profesional v1.0");
    w.resize(1200, 900);
    w.show();

    qDebug() << "\n✓ Sistema listo. Esperando interacción del usuario.\n";

    return a.exec();
}
