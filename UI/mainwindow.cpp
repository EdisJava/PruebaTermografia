#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_manager(new TermografiaManager(this))
{
    ui->setupUi(this);

    ui->labelImagen->setMouseTracking(true);
    ui->labelImagen->installEventFilter(this);

    // Conexiones de UI
    connect(ui->actionImportar_termogramas, &QAction::triggered,
            this, &MainWindow::on_actionImportar_termogramas_triggered);

    connect(ui->listTermogramas, &QListWidget::currentRowChanged,
            this, &MainWindow::seleccionarTermograma);

    connect(ui->btnImportar, &QPushButton::clicked,
        this, &MainWindow::on_actionImportar_termogramas_triggered);

    connect(ui->actionImportar_termogramas, &QAction::triggered,
        this, &MainWindow::on_actionImportar_termogramas_triggered);
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->labelImagen && event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        mostrarInfoPunto(mouseEvent->pos());
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::mostrarInfoPunto(QPoint pos) {
    Termograma* t = m_manager->obtenerTermogramaActivo();
    if (!t || t->matrizDatos.isEmpty() || m_currentPixmap.isNull()) return;

    // 1. Mapeo de coordenadas Pantalla -> Matriz de Datos (200x150)
    double factorX = static_cast<double>(t->ancho) / ui->labelImagen->width();
    double factorY = static_cast<double>(t->alto) / ui->labelImagen->height();

    int x = qBound(0, static_cast<int>(pos.x() * factorX), t->ancho - 1);
    int y = qBound(0, static_cast<int>(pos.y() * factorY), t->alto - 1);

    float temp = t->getTemperatura(x, y);

    // 2. Actualizar los QLineEdit inferiores (usando tus nombres reales del UI)
    ui->txtPixel->setText(QString("[%1, %2]").arg(x).arg(y));
    ui->txtTemp->setText(QString("%1 °C").arg(temp, 0, 'f', 1));

    // 3. Dibujar la mira dinámica sin parpadeo
    QPixmap overlay = m_currentPixmap.scaled(ui->labelImagen->size(), Qt::IgnoreAspectRatio);
    QPainter painter(&overlay);
    painter.setPen(QPen(Qt::yellow, 1));

    // Cruz de mira
    painter.drawLine(pos.x() - 8, pos.y(), pos.x() + 8, pos.y());
    painter.drawLine(pos.x(), pos.y() - 8, pos.x(), pos.y() + 8);

    // Etiqueta flotante
    painter.setBrush(QColor(0, 0, 0, 150));
    painter.drawRect(pos.x() + 10, pos.y() - 25, 60, 18);
    painter.setPen(Qt::white);
    painter.drawText(pos.x() + 12, pos.y() - 12, QString("%1°C").arg(temp, 0, 'f', 1));

    ui->labelImagen->setPixmap(overlay);
}

void MainWindow::dibujarCapaTermografica() {
    Termograma* t = m_manager->obtenerTermogramaActivo();
    if (!t || t->matrizDatos.isEmpty()) {
        qDebug() << "Error: No hay datos térmicos para dibujar.";
        return;
    }

    // 1. Encontrar Max y Min y sus posiciones exactas en la matriz
    float maxImg = -999.0f;
    float minImg = 999.0f;
    QPoint pMax(0,0), pMin(0,0);

    for (int y = 0; y < t->alto; ++y) {
        for (int x = 0; x < t->ancho; ++x) {
            float temp = t->getTemperatura(x, y);
            if (temp > maxImg) { maxImg = temp; pMax = QPoint(x, y); }
            if (temp < minImg) { minImg = temp; pMin = QPoint(x, y); }
        }
    }

    // Actualizar labels de la derecha
    ui->lblMaxTemp->setText(QString("Max: %1 °C").arg(maxImg, 0, 'f', 1));
    ui->lblMinTemp->setText(QString("Min: %1 °C").arg(minImg, 0, 'f', 1));

    // 2. Generar la imagen base
    QImage base = m_manager->generarImagenVisual(t->id, minImg, maxImg);
    if (base.isNull()) return;

    QPixmap pixmap = QPixmap::fromImage(base);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Dibujar marcadores Max (Rojo) y Min (Cian)
    painter.setPen(QPen(Qt::red, 1));
    painter.drawLine(pMax.x()-3, pMax.y(), pMax.x()+3, pMax.y());
    painter.drawLine(pMax.x(), pMax.y()-3, pMax.x(), pMax.y()+3);

    painter.setPen(QPen(Qt::cyan, 1));
    painter.drawLine(pMin.x()-3, pMin.y(), pMin.x()+3, pMin.y());
    painter.drawLine(pMin.x(), pMin.y()-3, pMin.x(), pMin.y()+3);

    // Hotspots automáticos
    AnalysisEngine engine;
    float umbralSugerencia = maxImg * 0.92f;
    QList<PuntoCaliente> puntos = engine.detectarPuntosCalientesAuto(*t, umbralSugerencia);

    painter.setPen(QPen(Qt::green, 1));
    for (const PuntoCaliente &p : std::as_const(puntos)) {
        painter.drawEllipse(QPoint(p.pixelX, p.pixelY), 2, 2);
    }
    painter.end();

    // GUARDADO CRÍTICO: Guardamos la imagen original 200x150 procesada
    m_currentPixmap = pixmap;

    // MOSTRAR: Estiramos la imagen para que llene TODO el label (IgnoreAspectRatio)
    ui->labelImagen->setPixmap(m_currentPixmap.scaled(ui->labelImagen->size(),
                                            Qt::IgnoreAspectRatio,
                                            Qt::SmoothTransformation));

    pintarLeyendaColores();
}

void MainWindow::pintarLeyendaColores() {
    int h = ui->labelRampa->height() > 0 ? ui->labelRampa->height() : 300;
    int w = ui->labelRampa->width() > 0 ? ui->labelRampa->width() : 50;

    QPixmap barra(w, h);
    QPainter p(&barra);
    QLinearGradient grad(0, h, 0, 0);
    grad.setColorAt(0.0, Qt::blue);
    grad.setColorAt(0.5, Qt::red);
    grad.setColorAt(1.0, Qt::white);

    p.fillRect(barra.rect(), grad);
    p.end();
    ui->labelRampa->setPixmap(barra);
}

void MainWindow::on_actionImportar_termogramas_triggered() {
    QString fileName = QFileDialog::getOpenFileName(this, "Abrir Manifiesto", "", "JSON (*.json)");
    if (fileName.isEmpty()) return;

    IngestionEngine::ResumenCarga res = m_manager->cargarLoteDesdeJSON(fileName);

    if (res.totalCargados > 0) {
        ui->listTermogramas->clear();

        // RECORRER LA LISTA CARGADA PARA USAR LOS NOMBRES DEL JSON
        for(int i = 0; i < res.totalCargados; ++i) {
            Termograma* t = m_manager->obtenerTermogramaActivo(i);
            if(t) {
                ui->listTermogramas->addItem(t->idAlternativo);
            }
        }
        ui->listTermogramas->setCurrentRow(0);
    }
}
void MainWindow::seleccionarTermograma(int index) {
    if (index < 0) return;

    // Obtenemos el termograma (el manager debe tener la lista cargada)
    Termograma* t = m_manager->obtenerTermogramaActivo(index);

    if (t) {
        // Mostramos las coordenadas del JSON en algún label o consola
        qDebug() << "Visualizando elemento:" << t->idAlternativo;
        qDebug() << "GPS:" << t->coordX << t->coordY << t->coordZ;

        // Aquí podrías actualizar un label de "Ubicación"
        // ui->lblGPS->setText(QString("X:%1 Y:%2").arg(t->coordX).arg(t->coordY));

        dibujarCapaTermografica();
    }
}
