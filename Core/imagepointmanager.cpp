#include "imagepointmanager.h"
#include <QDebug>

ImagePointManager::ImagePointManager(QObject *parent)
    : QObject(parent)
    , m_currentHotspot(nullptr)
    , m_nextId(1)
{
    qDebug() << "MetaObject class:" << metaObject()->className();

}

void ImagePointManager::setImage(const QImage& image)
{
    m_image = image;
    clearHotspots();
    emit imageChanged();
}

void ImagePointManager::addPixelToCurrentHotspot(const QPoint& pixel)
{
    qDebug() << "addPixelToCurrentHotspot called with pixel:" << pixel;

    if (!isValidCoordinate(pixel)) {
        qDebug() << "Invalid coordinate!";
        return;
    }

    qDebug() << "Creating new hotspot with ID:" << m_nextId;

    // Crear un nuevo hotspot para cada píxel
    ImagePoint newHotspot(m_nextId++);
    newHotspot.addPixel(pixel);
    m_hotspots.append(newHotspot);

    qDebug() << "Hotspot created. Total hotspots:" << m_hotspots.size();
    qDebug() << "Emitting hotspotUpdated signal";

    emit hotspotUpdated(m_hotspots.last());
}

void ImagePointManager::createNewHotspot()
{
    ImagePoint newHotspot(m_nextId++);
    m_hotspots.append(newHotspot);
    m_currentHotspot = &m_hotspots.last();
}

void ImagePointManager::clearHotspots()
{
    m_hotspots.clear();
    m_currentHotspot = nullptr;
    m_nextId = 1;
    emit hotspotsCleared();
}

bool ImagePointManager::isValidCoordinate(const QPoint& pixel) const
{
    if (m_image.isNull()) {
        return false;
    }

    return pixel.x() >= 0 && pixel.x() < m_image.width() &&
           pixel.y() >= 0 && pixel.y() < m_image.height();
}
