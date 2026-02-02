#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include "imagepoint.h"
#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QList>

class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWidget(QWidget *parent = nullptr);

    void setImage(const QImage& image);
    void setPixelMarkers(const QList<QPoint>& pixels);
    void clearPixelMarkers();

    QPoint screenToImageCoordinates(const QPoint& screenPos) const;
    QPoint imagePixelAt(const QPoint& widgetPos) const;

    void setHotspots(const QList<ImagePoint>& hotspots) {
        m_hotspots = hotspots;
        update(); // fuerza repintado
    }

signals:
    void rightClicked(const QPoint& imagePixel);
    void colorSelected(const QColor& color);
    void markHotspotRequested(const QPoint& pixel);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event);
private:
    QImage m_image;
    QList<QPoint> m_pixelMarkers;  // Píxeles marcados
    QList<ImagePoint> m_hotspots;
    QPoint m_lastRightClickPixel;


    QRectF getImageRect() const;
};

#endif // IMAGEWIDGET_H
