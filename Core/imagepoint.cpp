#include "imagepoint.h"

ImagePoint::ImagePoint()
    : m_id(0)
{
}

ImagePoint::ImagePoint(int id)
    : m_id(id)
{
}

void ImagePoint::addPixel(const QPoint& pixel)
{
    m_pixels.append(pixel);
}

QString ImagePoint::toString() const
{
    QString displayName = m_name.isEmpty() ? QString("Hotpoint%1").arg(m_id) : m_name;

    if (m_pixels.isEmpty()) {
        return QString("%1 - (vacío)").arg(displayName);
    }

    QStringList pixelStrings;
    for (const QPoint& pixel : m_pixels) {
        pixelStrings.append(QString("(%1,%2)").arg(pixel.x()).arg(pixel.y()));
    }

    return QString("%1 - %2").arg(displayName).arg(pixelStrings.join(", "));
}


void ImagePoint::setName(const QString& name) { m_name = name; }

QString ImagePoint::name() const { return m_name; }


