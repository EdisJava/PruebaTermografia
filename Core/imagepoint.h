#ifndef IMAGEPOINT_H
#define IMAGEPOINT_H

#include <QColor>
#include <QPoint>
#include <QString>
#include <QList>

// Clase simple que representa un punto caliente (hotspot) con sus píxeles
class ImagePoint
{
public:
    ImagePoint();
    explicit ImagePoint(int id);
    QColor m_color = Qt::red;

    // Añadir píxel al hotspot
    void addPixel(const QPoint& pixel);

    // Getters
    int id() const { return m_id; }
    QList<QPoint> pixels() const { return m_pixels; }
    int pixelCount() const { return m_pixels.size(); }
    QColor color() const { return m_color; }
    void setColor(const QColor& color) { m_color = color; }
    QString name() const;
    void setName(const QString& name);

    // Formato para mostrar
    QString toString() const;

private:
    int m_id;
    QList<QPoint> m_pixels;  // Lista de píxeles (x,y) de este hotspot
    QString m_name;
};

#endif // IMAGEPOINT_H
