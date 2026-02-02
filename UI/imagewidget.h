#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

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

signals:
    void rightClicked(const QPoint& imagePixel);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QImage m_image;
    QList<QPoint> m_pixelMarkers;  // Píxeles marcados

    QRectF getImageRect() const;
};

#endif // IMAGEWIDGET_H
