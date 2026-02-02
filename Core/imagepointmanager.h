#ifndef IMAGEPOINTMANAGER_H
#define IMAGEPOINTMANAGER_H

#include "imagepoint.h"
#include <QObject>
#include <QImage>
#include <QList>
#include <QPoint>

class ImagePointManager : public QObject
{
    Q_OBJECT

public:
    explicit ImagePointManager(QObject *parent = nullptr);

    // Gestión de imagen
    void setImage(const QImage& image);
    QImage image() const { return m_image; }
    bool hasImage() const { return !m_image.isNull(); }

    // Gestión de hotspots
    void addPixelToCurrentHotspot(const QPoint& pixel);
    void createNewHotspot();
    void clearHotspots();

    QList<ImagePoint> hotspots() const { return m_hotspots; }
    int hotspotCount() const { return m_hotspots.size(); }
    ImagePoint* currentHotspot() { return m_currentHotspot; }

    // Utilidad
    bool isValidCoordinate(const QPoint& pixel) const;
    void setCurrentHotspotColor(const QColor& color);

signals:
    void hotspotUpdated(const ImagePoint& hotspot);
    void hotspotsCleared();
    void imageChanged();
    void hotspotColorChanged();

private:
    QImage m_image;
    QList<ImagePoint> m_hotspots;
    ImagePoint* m_currentHotspot;
    int m_nextId;
    QColor m_globalHotspotColor = Qt::red;
};

#endif // IMAGEPOINTMANAGER_H
