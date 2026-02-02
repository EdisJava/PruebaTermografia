#include "engines.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>

IngestionEngine::IngestionEngine() {}

QList<Termograma> IngestionEngine::procesarManifiesto(const QString& jsonFile) {
    QList<Termograma> lista;

    QFile file(jsonFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Error: No se pudo abrir el archivo JSON:" << jsonFile;
        return lista;
    }

    QDir baseDir = QFileInfo(jsonFile).dir();
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull()) {
        qDebug() << "Error: JSON inválido o vacío";
        return lista;
    }

    QJsonObject root = doc.object();

    // 1. Procesar Apoyos
    if (root.contains("Apoyo")) {
        QJsonArray apoyos = root["Apoyo"].toArray();

        for (const QJsonValue& v : apoyos) {
            QJsonObject obj = v.toObject();
            QJsonObject termo = obj["termograma"].toObject();
            QJsonObject coords = termo["coordenada"].toObject();

            Termograma t;
            t.nombreArchivo = termo["url"].toString();
            t.rutaFisica = baseDir.absoluteFilePath(t.nombreArchivo);
            t.coordX = coords["x"].toDouble();
            t.coordY = coords["y"].toDouble();
            t.coordZ = coords["z"].toDouble();
            t.idAlternativo = "Apoyo " + obj["nombre"].toString();
            t.fechaCaptura = QDateTime::currentDateTime();

            // Cargar matriz térmica
            t.matrizDatos = parsearMatrizTermica(t.rutaFisica, t.ancho, t.alto);

            if (!t.matrizDatos.isEmpty()) {
                lista.append(t);
                qDebug() << "Apoyo cargado:" << t.idAlternativo;
            }
        }
    }

    // 2. Procesar Vanos
    if (root.contains("Vano")) {
        QJsonArray vanos = root["Vano"].toArray();

        for (const QJsonValue& v : vanos) {
            QJsonObject obj = v.toObject();
            QJsonObject termo = obj["termograma"].toObject();
            QJsonObject coords = termo["coordenada"].toObject();

            Termograma t;
            t.nombreArchivo = termo["url"].toString();
            t.rutaFisica = baseDir.absoluteFilePath(t.nombreArchivo);
            t.coordX = coords["x"].toDouble();
            t.coordY = coords["y"].toDouble();
            t.coordZ = coords["z"].toDouble();
            t.idAlternativo = "Vano " + obj["nombre"].toString();
            t.fechaCaptura = QDateTime::currentDateTime();

            // Cargar matriz térmica
            t.matrizDatos = parsearMatrizTermica(t.rutaFisica, t.ancho, t.alto);

            if (!t.matrizDatos.isEmpty()) {
                lista.append(t);
                qDebug() << "Vano cargado:" << t.idAlternativo;
            }
        }
    }

    qDebug() << "Total de termogramas cargados:" << lista.size();
    return lista;
}

QVector<float> IngestionEngine::parsearMatrizTermica(const QString& rutaCSV, int& w, int& h) {
    QVector<float> matriz;

    QFile file(rutaCSV);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: No se pudo abrir el archivo CSV:" << rutaCSV;
        return matriz;
    }

    QTextStream in(&file);
    int filas = 0;
    int columnas = 0;

    // Leer línea por línea
    while (!in.atEnd()) {
        QString linea = in.readLine().trimmed();

        // Saltar líneas vacías
        if (linea.isEmpty()) continue;

        // Dividir por comas
        QStringList valores = linea.split(",", Qt::SkipEmptyParts);

        // Primera fila determina el número de columnas
        if (columnas == 0) {
            columnas = valores.size();
        }

        // Convertir cada valor a float
        for (const QString& val : valores) {
            bool ok;
            float temp = val.trimmed().toFloat(&ok);

            if (ok) {
                matriz.append(temp);
            } else {
                qDebug() << "Advertencia: Valor no numérico encontrado:" << val;
                matriz.append(0.0f); // Valor por defecto
            }
        }

        filas++;
    }

    file.close();

    // Actualizar dimensiones
    w = columnas;
    h = filas;

    qDebug() << "CSV procesado:" << rutaCSV
             << "| Dimensiones:" << w << "x" << h
             << "| Total valores:" << matriz.size();

    // Validación
    if (matriz.size() != w * h) {
        qDebug() << "Advertencia: Inconsistencia en dimensiones de matriz";
    }

    return matriz;
}
