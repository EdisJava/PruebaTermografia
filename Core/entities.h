#ifndef ENTITIES_H
#define ENTITIES_H

#include <QString>
#include <QVector>
#include <QDateTime>
#include <QList>

enum class TipoPunto { MANUAL, SUGERIDO };
enum class TipoElemento { APOYO, VANO };

class PuntoCaliente {
public:
    long id;
    float temperaturaMax;
    int pixelX;
    int pixelY;
    TipoPunto tipo;
    bool validado;
    QString comentario;

    PuntoCaliente() : id(-1), temperaturaMax(0.0f), pixelX(0), pixelY(0),
                      tipo(TipoPunto::MANUAL), validado(false) {}
};

class Termograma {
public:
    long id;
    QString nombreArchivo;
    QString rutaFisica;
    QString idAlternativo;
    QDateTime fechaCaptura;
    double coordX, coordY, coordZ;
    int ancho;
    int alto;
    QVector<float> matrizDatos;
    QList<PuntoCaliente> listaPuntos;

    Termograma() : id(-1), ancho(0), alto(0) {}

    float getTemperatura(int x, int y) const {
        if (x < 0 || x >= ancho || y < 0 || y >= alto || matrizDatos.isEmpty()) return 0.0f;
        return matrizDatos[y * ancho + x];
    }
};

class ElementoRed {
public:
    long id;
    QString codigo;
    TipoElemento tipo;
    QList<Termograma> listaTermogramas;
};

#endif
