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
    if (m_pixels.isEmpty()) {
        return QString("Hotpoint%1 - (vacío)").arg(m_id);
    }

    QStringList pixelStrings;
    for (const QPoint& pixel : m_pixels) {
        pixelStrings.append(QString("(%1,%2)").arg(pixel.x()).arg(pixel.y()));
    }

    return QString("Hotpoint%1 - %2").arg(m_id).arg(pixelStrings.join(", "));
}
