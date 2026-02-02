#include "engines.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>

IngestionEngine::IngestionEngine() {}

QList<Termograma> IngestionEngine::procesarManifiesto(const QString& jsonFile) {
    QList<Termograma> lista;
    QFile file(jsonFile);
    if (!file.open(QIODevice::ReadOnly)) return lista;

    QDir baseDir = QFileInfo(jsonFile).dir();
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    // 1. Procesar Apoyos
    QJsonArray apoyos = root["Apoyo"].toArray();
    for (QJsonValue v : apoyos) {
        QJsonObject obj = v.toObject();
        QJsonObject termo = obj["termograma"].toObject();
        QJsonObject coords = termo["coordenada"].toObject();

        Termograma t;
        t.nombreArchivo = termo["url"].toString();
        t.rutaFisica = baseDir.absoluteFilePath(t.nombreArchivo);
        t.coordX = coords["x"].toDouble();
        t.coordY = coords["y"].toDouble();
        t.coordZ = coords["z"].toDouble();
        // Guardamos el nombre del apoyo para la lista
        t.idAlternativo = "Apoyo " + obj["nombre"].toString();

        t.matrizDatos = parsearMatrizTermica(t.rutaFisica, t.ancho, t.alto);
        if(!t.matrizDatos.isEmpty()) lista.append(t);
    }

    // 2. Procesar Vanos
    QJsonArray vanos = root["Vano"].toArray();
    for (QJsonValue v : vanos) {
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

        t.matrizDatos = parsearMatrizTermica(t.rutaFisica, t.ancho, t.alto);
        if(!t.matrizDatos.isEmpty()) lista.append(t);
    }

    return lista;
}

QVector<float> IngestionEngine::parsearMatrizTermica(const QString& rutaCSV, int& w, int& h) {
    QVector<float> matriz;
    QFile file(rutaCSV);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "No se pudo abrir el CSV:" << rutaCSV;
        return matriz;
    }

    QTextStream in(&file);
    int filas = 0, columnas = 0;

    while (!in.atEnd()) {
        QString linea = in.readLine().trimmed();
        if (linea.isEmpty()) continue;

        // CAMBIO: split por COMA "," que es lo que genera el main.cpp
        QStringList valores = linea.split(",", Qt::SkipEmptyParts);

        if (columnas == 0) columnas = valores.size();

        for (const QString& val : valores) {
            // Limpiamos espacios y convertimos a float
            matriz.append(val.trimmed().toFloat());
        }
        filas++;
    }

    // Solo actualizamos w y h si no venían ya definidos (o para verificar)
    w = columnas;
    h = filas;

    qDebug() << "CSV cargado:" << rutaCSV << "Dimensiones:" << w << "x" << h;
    return matriz;
}
