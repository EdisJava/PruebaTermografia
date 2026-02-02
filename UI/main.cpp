#include "mainwindow.h"
#include <QApplication>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <fstream>
#include <cmath>


// Función para generar los archivos CSV de 200x150
void generarPrueba(std::string nombre, int w, int h, float tempBase, int hotX, int hotY) {
    std::ofstream file(nombre);
    if(!file.is_open()) return;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float temp = tempBase + (static_cast<float>(rand()) / RAND_MAX) * 2.0f;
            float dist = std::sqrt(std::pow(x - hotX, 2) + std::pow(y - hotY, 2));
            if (dist < 15) {
                temp += 60.0f * std::exp(-dist / 5.0f);
            }
            file << temp << (x == w - 1 ? "" : ",");
        }
        file << "\n";
    }
    file.close();
    qDebug() << "Archivo generado:" << QString::fromStdString(nombre);
}

// Función para inicializar la base de datos de DALIA
bool inicializarBaseDeDatos() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("dalia_termografia.db");

    if (!db.open()) {
        qDebug() << "Error abriendo base de datos:" << db.lastError().text();
        return false;
    }

    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS termogramas ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "nombre_archivo TEXT, ruta_fisica TEXT, fecha DATETIME, "
           "x REAL, y REAL, z REAL, ancho INTEGER, alto INTEGER)");

    q.exec("CREATE TABLE IF NOT EXISTS puntos_calientes ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "id_termograma INTEGER, temp_max REAL, x INTEGER, y INTEGER, "
           "tipo INTEGER, comentario TEXT, validado BOOLEAN, "
           "FOREIGN KEY(id_termograma) REFERENCES termogramas(id))");

    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. Crear los archivos de datos físicos (200x150)
    // Se crearán en la carpeta donde se ejecute el .exe
    generarPrueba("data_200x150_1.csv", 200, 150, 25.0, 50, 40);
    generarPrueba("data_200x150_2.csv", 200, 150, 40.0, 150, 100);

    if (!inicializarBaseDeDatos()) {
        return -1;
    }

    MainWindow w;
    w.show();
    return a.exec();
}

