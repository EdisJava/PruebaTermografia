#include "imagewidget.h"
#include <QPainter>
#include <QMouseEvent>

ImageWidget::ImageWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setMouseTracking(true);
}

void ImageWidget::setImage(const QImage& image)
{
    m_image = image;
    clearPixelMarkers();
    update();
}

void ImageWidget::setPixelMarkers(const QList<QPoint>& pixels)
{
    m_pixelMarkers = pixels;
    update();
}

void ImageWidget::clearPixelMarkers()
{
    m_pixelMarkers.clear();
    update();
}

QPoint ImageWidget::screenToImageCoordinates(const QPoint& screenPos) const
{
    if (m_image.isNull()) {
        return QPoint();
    }

    QRectF imageRect = getImageRect();

    // Convertir posición de pantalla a coordenadas de imagen
    double relX = (screenPos.x() - imageRect.x()) / imageRect.width();
    double relY = (screenPos.y() - imageRect.y()) / imageRect.height();

    int imgX = qRound(relX * m_image.width());
    int imgY = qRound(relY * m_image.height());

    return QPoint(imgX, imgY);
}


/**
 * Ejemplo:
 *  imagen de 1000x800
 *  Widget en pantalla 500x600
 *  Queremos dibujar el pixel en las coordenadas 750,400 de la imagen
 *
 * widgetAspect = 500/600 = 0.833  (más alto que ancho)
 * imageAspect = 1000/800 = 1.25   (más ancho que alto)
 *
 * tamañowidget es menor a tamaño imagen
 *  imagen mas ancha en proporcion
 *  se ajusta el ancho del widgets y sobran franjas arriba y abajo por el tamaño
 *
 *
 *  CONVERTIR  PIXEL IMAGEN A POSICION REALTIVA
 *  pixel = (750, 400) imagen original
 *
 *  relX = (double)pixel.x() / m_image.width();
 *  relX = 750 / 1000 = 0.75  ← El píxel está al 75% del ancho
 *
 *  relY = (double)pixel.y() / m_image.height();
 *  relY = 400 / 800 = 0.5    ← El píxel está al 50% del alto (centro)
 *
 *
 *  CONVERTIR POSICION RELAIVA A COORDS DE PANTALLA
 *  screenX = imageRect.x() + relX * imageRect.width();
 *  screenX = 0 + 0.75 * 500 = 375  ← 375 píxeles desde la izquierda del widget
 *
 *  screenY = imageRect.y() + relY * imageRect.height();
 *  screenY = 100 + 0.5 * 400 = 300  ← 300 píxeles desde arriba del widget

 * */


void ImageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), Qt::darkGray);

    if (m_image.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "Carga una imagen (Archivo > Abrir)");
        return;
    }

    // Dibujar imagen escalada manteniendo aspecto
    QRectF imageRect = getImageRect();
    painter.drawImage(imageRect, m_image);

    // Dibujar píxeles marcados
    painter.setPen(QPen(Qt::red, 2));
    painter.setBrush(Qt::red);

    for (const QPoint& pixel : m_pixelMarkers) {
        // Convertir coordenadas de imagen a coordenadas de pantalla
        double relX = (double)pixel.x() / m_image.width();
        double relY = (double)pixel.y() / m_image.height();

        double screenX = imageRect.x() + relX * imageRect.width();
        double screenY = imageRect.y() + relY * imageRect.height();

        painter.drawEllipse(QPointF(screenX, screenY), 3, 3);
    }
}

void ImageWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton && !m_image.isNull()) {
        QPoint pixel = screenToImageCoordinates(event->pos());

        // Verificar que el click está dentro de la imagen
        if (pixel.x() >= 0 && pixel.x() < m_image.width() &&
            pixel.y() >= 0 && pixel.y() < m_image.height()) {
            emit rightClicked(pixel);
        }
    }

    QWidget::mousePressEvent(event);
}

QRectF ImageWidget::getImageRect() const
{
    if (m_image.isNull()) {
        return QRectF();
    }

    // Calcular rectángulo para mantener aspecto de la imagen
    double widgetAspect = (double)width() / height();
    double imageAspect = (double)m_image.width() / m_image.height();

    QRectF imageRect;

    if (widgetAspect > imageAspect) {
        // Widget más ancho que la imagen
        double scaledWidth = height() * imageAspect;
        imageRect = QRectF((width() - scaledWidth) / 2, 0, scaledWidth, height());
    } else {
        // Widget más alto que la imagen
        double scaledHeight = width() / imageAspect;
        //imageRect= QRectF(x, y, width(), scaledHeight);
        imageRect = QRectF(0, (height() - scaledHeight) / 2, width(), scaledHeight);
    }

    return imageRect;
}
