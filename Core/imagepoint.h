#ifndef IMAGEPOINT_H
#define IMAGEPOINT_H

#include <QPoint>
#include <QString>
#include <QList>

// Clase simple que representa un punto caliente (hotspot) con sus píxeles
class ImagePoint
{
public:
    ImagePoint();
    explicit ImagePoint(int id);

    // Añadir píxel al hotspot
    void addPixel(const QPoint& pixel);

    // Getters
    int id() const { return m_id; }
    QList<QPoint> pixels() const { return m_pixels; }
    int pixelCount() const { return m_pixels.size(); }

    // Formato para mostrar
    QString toString() const;

private:
    int m_id;
    QList<QPoint> m_pixels;  // Lista de píxeles (x,y) de este hotspot
};

#endif // IMAGEPOINT_H
