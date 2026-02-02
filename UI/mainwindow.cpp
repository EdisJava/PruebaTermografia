/**
 * @file mainwindow.cpp
 * @brief Implementación de la ventana principal de DALIA
 *
 * Sistema de visualización termográfica con soporte para:
 * - Visualización de mapas de calor en tiempo real
 * - Marcado manual de puntos de interés
 * - Análisis interactivo de temperaturas
 * - Gestión de múltiples termogramas
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMouseEvent>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>
#include <QMenu>
#include <QDebug>
#include <QColor>
#include <QLinearGradient>
#include <QTextStream>
#include <QPainterPath>
#include <QToolTip>
#include <QtMath>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_manager(new TermografiaManager(this))
    , m_opacidadTermica(0.7)
    , m_umbralAlarma(60.0f)
    , m_mostrarMaximoAuto(true)
    , m_puntoMaximoX(-1)
    , m_puntoMaximoY(-1)
    , m_puntoMaximoTemp(0.0f)
{
    ui->setupUi(this);

    qDebug() << "Inicializando MainWindow...";

    // =================================================================
    // CONFIGURACIÓN DE COMPONENTES UI
    // =================================================================

    // Label de imagen: mantener proporción al redimensionar
    ui->labelImagen->setScaledContents(false);
    ui->labelImagen->setAlignment(Qt::AlignCenter);
    ui->labelImagen->setMouseTracking(true);
    ui->labelImagen->installEventFilter(this);

    // Label de rampa de colores: permitir escalado automático
    ui->labelRampa->setScaledContents(true);

    // =================================================================
    // CONFIGURACIÓN DE CONTROLES
    // =================================================================

    // Slider de opacidad
    ui->sliderOpacidad->setRange(0, 100);
    ui->sliderOpacidad->setValue(70);

    // =================================================================
    // CONEXIONES DE SEÑALES Y SLOTS
    // =================================================================

    // Botón importar
    connect(ui->btnImportar, &QPushButton::clicked,
            this, &MainWindow::on_actionImportar_termogramas_triggered);

    // Lista de termogramas
    connect(ui->listTermogramas, &QListWidget::currentRowChanged,
            this, &MainWindow::seleccionarTermograma);

    // Slider de opacidad
    connect(ui->sliderOpacidad, &QSlider::valueChanged, this, [this](int v) {
        m_opacidadTermica = v / 100.0;
        ui->lblOpacidadVal->setText(QString("%1%").arg(v));
        dibujarCapaTermografica();
    });

    // =================================================================
    // TIMER PARA BARRA DE ESTADO
    // =================================================================

    m_timerEstado = new QTimer(this);
    connect(m_timerEstado, &QTimer::timeout, this, &MainWindow::actualizarRelojEstado);
    m_timerEstado->start(1000);

    // =================================================================
    // INICIALIZACIÓN FINAL
    // =================================================================

    pintarLeyendaColores();
    statusBar()->showMessage("✓ Sistema DALIA iniciado correctamente", 3000);

    qDebug() << "MainWindow inicializado correctamente";
}

MainWindow::~MainWindow() {
    delete ui;
    qDebug() << "MainWindow destruido";
}

// =============================================================================
// SECCIÓN: RENDERIZADO Y VISUALIZACIÓN
// =============================================================================

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    // Redibujar al cambiar tamaño de ventana
    if (m_manager->obtenerTermogramaActivo()) {
        dibujarCapaTermografica();
    }
}

void MainWindow::dibujarCapaTermografica() {
    Termograma* t = m_manager->obtenerTermogramaActivo();

    if (!t || t->matrizDatos.isEmpty()) {
        ui->labelImagen->clear();
        ui->labelImagen->setText("No hay datos termográficos cargados");
        return;
    }

    // =========================================================================
    // PASO 1: CALCULAR ESTADÍSTICAS TÉRMICAS
    // =========================================================================

    float maxT = -999.0f;
    float minT = 999.0f;

    for (float v : t->matrizDatos) {
        if (v > maxT) maxT = v;
        if (v < minT) minT = v;
    }

    // Actualizar etiquetas
    ui->lblMaxTemp->setText(QString("MAX: %1 °C").arg(maxT, 0, 'f', 1));
    ui->lblMinTemp->setText(QString("MIN: %1 °C").arg(minT, 0, 'f', 1));

    // =========================================================================
    // PASO 2: CREAR CANVAS EN RESOLUCIÓN NATIVA
    // =========================================================================

    QImage canvas(t->ancho, t->alto, QImage::Format_ARGB32);
    canvas.fill(Qt::black);

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    // =========================================================================
    // CAPA A: IMAGEN DE FONDO (SI EXISTE)
    // =========================================================================

    QString pathImg = t->rutaFisica;
    pathImg.replace(".csv", ".jpg").replace(".json", ".jpg").replace(".txt", ".jpg");

    QImage imgFondo(pathImg);
    if (!imgFondo.isNull()) {
        painter.drawImage(canvas.rect(), imgFondo);
    }

    // =========================================================================
    // CAPA B: MÁSCARA TÉRMICA CON OPACIDAD
    // =========================================================================

    QImage thermalMask = m_manager->generarImagenVisual(t->id, minT, maxT);

    if (!thermalMask.isNull()) {
        painter.setOpacity(m_opacidadTermica);
        painter.drawImage(0, 0, thermalMask);
        painter.setOpacity(1.0);
    }

    // =========================================================================
    // CAPA C: ELEMENTOS VECTORIALES (PUNTOS Y ANOTACIONES)
    // =========================================================================

    // C.1 - Puntos manuales marcados por el usuario (PRECISIÓN MEJORADA)
    for (const auto& p : t->listaPuntos) {
        // Punto PEQUEÑO y PRECISO para análisis de componentes electrónicos
        // Círculo blanco exterior (borde de contraste)
        painter.setPen(QPen(Qt::white, 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(p.pixelX, p.pixelY), 2.0, 2.0);

        // Círculo amarillo interior (punto de marcado)
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 0));
        painter.drawEllipse(QPointF(p.pixelX, p.pixelY), 1.5, 1.5);

        // Cruz de precisión (opcional, muy fina)
        painter.setPen(QPen(Qt::white, 0.5));
        painter.drawLine(QPointF(p.pixelX - 3, p.pixelY), QPointF(p.pixelX + 3, p.pixelY));
        painter.drawLine(QPointF(p.pixelX, p.pixelY - 3), QPointF(p.pixelX, p.pixelY + 3));

        // Etiqueta SOLO visible cuando hay espacio (para no saturar)
        // En trabajo de precisión, es mejor mostrar tooltip
        if (t->listaPuntos.size() <= 5) {  // Solo si hay pocos puntos
            // Preparar texto de la etiqueta (compacto)
            QString texto = QString("%1: %2°")
                            .arg(p.comentario)
                            .arg(p.temperaturaMax, 0, 'f', 1);

            // Fuente pequeña y legible
            QFont font("Arial", 6);
            painter.setFont(font);
            QFontMetrics fm(font);

            // Calcular dimensiones
            int textWidth = fm.horizontalAdvance(texto) + 6;
            int textHeight = fm.height() + 2;

            // Fondo compacto
            painter.fillRect(p.pixelX + 5, p.pixelY - textHeight - 1,
                            textWidth, textHeight,
                            QColor(0, 0, 0, 200));

            // Texto amarillo
            painter.setPen(QColor(255, 255, 0));
            painter.drawText(p.pixelX + 8, p.pixelY - 3, texto);
        }
    }

    // C.2 - Punto de temperatura máxima automático (ULTRA PRECISO)
    if (m_mostrarMaximoAuto) {
        // Buscar posición del máximo
        for (int y = 0; y < t->alto; ++y) {
            for (int x = 0; x < t->ancho; ++x) {
                float temp = t->getTemperatura(x, y);

                if (qAbs(temp - maxT) < 0.01f) {
                    // Guardar posición del punto máximo para tooltip
                    m_puntoMaximoX = x;
                    m_puntoMaximoY = y;
                    m_puntoMaximoTemp = maxT;

                    // Punto MINIMALISTA de alta precisión
                    // Círculo exterior blanco muy fino (contraste)
                    painter.setPen(QPen(Qt::white, 1.2));
                    painter.setBrush(Qt::NoBrush);
                    painter.drawEllipse(QPointF(x, y), 2.5, 2.5);

                    // Círculo rojo brillante ultra pequeño
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor(255, 30, 30)); // Rojo intenso
                    painter.drawEllipse(QPointF(x, y), 2.0, 2.0);

                    // Punto central blanco (píxel exacto)
                    painter.setBrush(Qt::white);
                    painter.drawEllipse(QPointF(x, y), 0.8, 0.8);

                    goto encontrado_maximo;
                }
            }
        }
    }
    encontrado_maximo:

    painter.end();

    // =========================================================================
    // PASO 3: ESCALAR Y MOSTRAR EN EL LABEL
    // =========================================================================

    m_currentPixmap = QPixmap::fromImage(canvas);

    // Escalar manteniendo proporción
    QPixmap displayMap = m_currentPixmap.scaled(
        ui->labelImagen->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    ui->labelImagen->setPixmap(displayMap);
}

void MainWindow::pintarLeyendaColores() {
    // Crear rampa de color fija (será escalada automáticamente)
    QImage ramp(1, 512, QImage::Format_RGB32);
    QPainter p(&ramp);

    // Gradiente térmico: Azul → Cian → Verde → Amarillo → Rojo
    QLinearGradient gradient(0, 512, 0, 0);
    gradient.setColorAt(0.0, QColor(0, 0, 255));     // Azul
    gradient.setColorAt(0.25, QColor(0, 255, 255));  // Cian
    gradient.setColorAt(0.5, QColor(0, 255, 0));     // Verde
    gradient.setColorAt(0.75, QColor(255, 255, 0));  // Amarillo
    gradient.setColorAt(1.0, QColor(255, 0, 0));     // Rojo

    p.fillRect(ramp.rect(), gradient);
    p.end();

    ui->labelRampa->setPixmap(QPixmap::fromImage(ramp));
}

// =============================================================================
// SECCIÓN: INTERACCIÓN CON EL USUARIO
// =============================================================================

QPoint MainWindow::getCoordenadaImagen(QPoint posMouse) {
    if (m_currentPixmap.isNull() || ui->labelImagen->pixmap().isNull()) {
        return QPoint(-1, -1);
    }

    // Calcular offset del pixmap dentro del label (por centrado)
    QSize labelSize = ui->labelImagen->size();
    QSize pixmapSize = ui->labelImagen->pixmap().size();

    int offsetX = (labelSize.width() - pixmapSize.width()) / 2;
    int offsetY = (labelSize.height() - pixmapSize.height()) / 2;

    // Coordenada relativa al inicio de la imagen
    int x_rel = posMouse.x() - offsetX;
    int y_rel = posMouse.y() - offsetY;

    // Obtener dimensiones originales
    Termograma* t = m_manager->obtenerTermogramaActivo();
    if (!t) return QPoint(-1, -1);

    // Calcular factores de escala
    double factorX = static_cast<double>(t->ancho) / pixmapSize.width();
    double factorY = static_cast<double>(t->alto) / pixmapSize.height();

    // Coordenadas en la imagen original
    int finalX = static_cast<int>(x_rel * factorX);
    int finalY = static_cast<int>(y_rel * factorY);

    // Validar límites
    if (finalX < 0 || finalX >= t->ancho || finalY < 0 || finalY >= t->alto) {
        return QPoint(-1, -1);
    }

    return QPoint(finalX, finalY);
}

void MainWindow::mostrarInfoPunto(QPoint pos) {
    Termograma* t = m_manager->obtenerTermogramaActivo();
    QPoint realPos = getCoordenadaImagen(pos);

    if (realPos.x() < 0 || !t) {
        ui->txtPixel->setText("—");
        ui->txtTemp->setText("—");
        ui->labelImagen->setToolTip("");
        return;
    }

    float temp = t->getTemperatura(realPos.x(), realPos.y());

    // Primero: Verificar si está cerca del punto máximo
    bool sobrePuntoMaximo = false;
    if (m_puntoMaximoX >= 0 && m_puntoMaximoY >= 0) {
        int dx = realPos.x() - m_puntoMaximoX;
        int dy = realPos.y() - m_puntoMaximoY;
        int distancia = qSqrt(dx * dx + dy * dy);

        if (distancia <= 6) {  // Radio reducido para precisión
            sobrePuntoMaximo = true;
        }
    }

    // Segundo: Verificar si está cerca de algún punto manual
    PuntoCaliente* puntoManualCercano = nullptr;
    int distanciaMinima = 9999;

    for (auto& p : t->listaPuntos) {
        int dx = realPos.x() - p.pixelX;
        int dy = realPos.y() - p.pixelY;
        int distancia = qSqrt(dx * dx + dy * dy);

        if (distancia <= 6 && distancia < distanciaMinima) {  // Radio 6px
            puntoManualCercano = &p;
            distanciaMinima = distancia;
        }
    }

    if (sobrePuntoMaximo) {
        // Mostrar información del punto máximo
        ui->txtPixel->setText(QString("(%1, %2) [MAX]").arg(m_puntoMaximoX).arg(m_puntoMaximoY));
        ui->txtTemp->setText(QString("%1 °C").arg(m_puntoMaximoTemp, 0, 'f', 2));

        QString tooltip = QString(
            "<div style='background-color: #c0392b; color: white; padding: 8px; border-radius: 4px;'>"
            "<b style='font-size: 14px;'>🔥 Punto Máximo</b><br>"
            "<span style='font-size: 18px; font-weight: bold;'>%1 °C</span><br>"
            "<span style='font-size: 11px;'>Coordenadas: (%2, %3)</span>"
            "</div>"
        ).arg(m_puntoMaximoTemp, 0, 'f', 1).arg(m_puntoMaximoX).arg(m_puntoMaximoY);

        ui->labelImagen->setToolTip(tooltip);

    } else if (puntoManualCercano) {
        // Mostrar información del punto manual
        ui->txtPixel->setText(QString("(%1, %2) [%3]")
                             .arg(puntoManualCercano->pixelX)
                             .arg(puntoManualCercano->pixelY)
                             .arg(puntoManualCercano->comentario));
        ui->txtTemp->setText(QString("%1 °C").arg(puntoManualCercano->temperaturaMax, 0, 'f', 2));

        QString tooltip = QString(
            "<div style='background-color: #f39c12; color: white; padding: 8px; border-radius: 4px;'>"
            "<b style='font-size: 14px;'>📍 %1</b><br>"
            "<span style='font-size: 16px; font-weight: bold;'>%2 °C</span><br>"
            "<span style='font-size: 11px;'>Posición: (%3, %4)</span>"
            "</div>"
        ).arg(puntoManualCercano->comentario)
         .arg(puntoManualCercano->temperaturaMax, 0, 'f', 1)
         .arg(puntoManualCercano->pixelX)
         .arg(puntoManualCercano->pixelY);

        ui->labelImagen->setToolTip(tooltip);

    } else {
        // Información normal del píxel bajo el cursor
        ui->txtPixel->setText(QString("(%1, %2)").arg(realPos.x()).arg(realPos.y()));
        ui->txtTemp->setText(QString("%1 °C").arg(temp, 0, 'f', 2));
        ui->labelImagen->setToolTip("");
    }
}

void MainWindow::marcarPuntoManual(QPoint pos) {
    Termograma* t = m_manager->obtenerTermogramaActivo();
    QPoint realPos = getCoordenadaImagen(pos);

    if (realPos.x() < 0 || !t) {
        QMessageBox::warning(this, "Error", "Coordenada inválida");
        return;
    }

    // Solicitar nombre al usuario
    bool ok;
    QString nombre = QInputDialog::getText(
        this,
        "Marcar Punto de Análisis",
        "Ingrese un nombre identificativo:",
        QLineEdit::Normal,
        QString("P%1").arg(t->listaPuntos.size() + 1),
        &ok
    );

    if (ok && !nombre.isEmpty()) {
        PuntoCaliente p;
        p.pixelX = realPos.x();
        p.pixelY = realPos.y();
        p.temperaturaMax = t->getTemperatura(realPos.x(), realPos.y());
        p.comentario = nombre;
        p.tipo = TipoPunto::MANUAL;
        p.validado = true;

        t->listaPuntos.append(p);

        qDebug() << "Punto marcado:" << nombre
                 << "en" << realPos
                 << "con temperatura" << p.temperaturaMax << "°C";

        dibujarCapaTermografica();
    }
}

void MainWindow::eliminarPuntoCercano(QPoint pos) {
    Termograma* t = m_manager->obtenerTermogramaActivo();
    QPoint realPos = getCoordenadaImagen(pos);

    if (realPos.x() < 0 || !t) return;

    // Buscar punto cercano (radio de 8 píxeles)
    const int RADIO_BUSQUEDA = 8;

    for (int i = 0; i < t->listaPuntos.size(); ++i) {
        int dx = t->listaPuntos[i].pixelX - realPos.x();
        int dy = t->listaPuntos[i].pixelY - realPos.y();
        int distancia = qSqrt(dx * dx + dy * dy);

        if (distancia <= RADIO_BUSQUEDA) {
            QString nombre = t->listaPuntos[i].comentario;
            t->listaPuntos.removeAt(i);

            qDebug() << "Punto eliminado:" << nombre;
            statusBar()->showMessage(QString("Punto '%1' eliminado").arg(nombre), 2000);

            dibujarCapaTermografica();
            return;
        }
    }

    statusBar()->showMessage("No hay puntos cercanos para eliminar", 2000);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->labelImagen) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        // Movimiento del ratón: mostrar info en tiempo real
        if (event->type() == QEvent::MouseMove) {
            mostrarInfoPunto(mouseEvent->pos());
            return true;
        }

        // Click derecho: menú contextual
        if (event->type() == QEvent::MouseButtonPress &&
            mouseEvent->button() == Qt::RightButton) {

            QMenu menu(this);
            menu.setStyleSheet("QMenu { background-color: white; padding: 5px; }"
                             "QMenu::item { padding: 8px 20px; }"
                             "QMenu::item:selected { background-color: #3498db; color: white; }");

            QAction *actAnalizar = menu.addAction("➕ Analizar este punto");
            QAction *actBorrar = menu.addAction("❌ Borrar punto cercano");
            menu.addSeparator();
            QAction *actCancelar = menu.addAction("↩️ Cancelar");

            QAction *selected = menu.exec(mouseEvent->globalPosition().toPoint());

            if (selected == actAnalizar) {
                marcarPuntoManual(mouseEvent->pos());
            } else if (selected == actBorrar) {
                eliminarPuntoCercano(mouseEvent->pos());
            }

            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

// =============================================================================
// SECCIÓN: IMPORTACIÓN Y GESTIÓN DE DATOS
// =============================================================================

void MainWindow::on_actionImportar_termogramas_triggered() {
    QString path = QFileDialog::getOpenFileName(
        this,
        "Seleccionar archivo JSON de dataset",
        QDir::currentPath(),
        "Archivos JSON (*.json);;Todos los archivos (*.*)"
    );

    if (path.isEmpty()) return;

    qDebug() << "Importando dataset desde:" << path;
    statusBar()->showMessage("Cargando dataset...");

    IngestionEngine::ResumenCarga resultado = m_manager->cargarLoteDesdeJSON(path);

    if (resultado.totalCargados > 0) {
        // Actualizar lista visual
        ui->listTermogramas->clear();

        for (int i = 0; i < resultado.totalCargados; ++i) {
            Termograma* t = m_manager->obtenerTermogramaActivo(i);
            if (t) {
                ui->listTermogramas->addItem(t->idAlternativo);
            }
        }

        // Seleccionar el primero automáticamente
        ui->listTermogramas->setCurrentRow(0);

        QString mensaje = QString("✓ Cargados %1 termogramas correctamente")
                         .arg(resultado.totalCargados);

        if (resultado.errores > 0) {
            mensaje += QString(" (%1 errores)").arg(resultado.errores);
        }

        statusBar()->showMessage(mensaje, 5000);

        QMessageBox::information(this, "Importación Exitosa", mensaje);
    } else {
        QString mensaje = "✗ No se pudieron cargar termogramas";
        statusBar()->showMessage(mensaje, 5000);
        QMessageBox::warning(this, "Error", mensaje);
    }
}

void MainWindow::seleccionarTermograma(int index) {
    if (index >= 0) {
        m_manager->setIndiceActivo(index);

        Termograma* t = m_manager->obtenerTermogramaActivo();
        if (t) {
            qDebug() << "Termograma seleccionado:" << t->idAlternativo;
            statusBar()->showMessage(QString("Visualizando: %1").arg(t->idAlternativo));
        }

        dibujarCapaTermografica();
    }
}

// =============================================================================
// SECCIÓN: FUNCIONES AUXILIARES
// =============================================================================

void MainWindow::on_btnVerLista_clicked() {
    Termograma* t = m_manager->obtenerTermogramaActivo();

    if (!t) {
        QMessageBox::information(this, "Inventario", "No hay termograma activo");
        return;
    }

    if (t->listaPuntos.isEmpty()) {
        QMessageBox::information(this, "Inventario Crítico",
                               "No hay puntos marcados en este termograma");
        return;
    }

    QString lista = QString("Puntos marcados en '%1':\n\n").arg(t->idAlternativo);

    for (int i = 0; i < t->listaPuntos.size(); ++i) {
        const auto& p = t->listaPuntos[i];
        lista += QString("%1. %2 - %3°C (x:%4, y:%5)\n")
                .arg(i + 1)
                .arg(p.comentario)
                .arg(p.temperaturaMax, 0, 'f', 1)
                .arg(p.pixelX)
                .arg(p.pixelY);
    }

    QMessageBox::information(this, "Inventario Crítico", lista);
}

void MainWindow::actualizarRelojEstado() {
    QString hora = QDateTime::currentDateTime().toString("HH:mm:ss");
    statusBar()->showMessage(QString("DALIA System | %1").arg(hora));
}

// Funciones placeholder para futuras implementaciones
void MainWindow::calcularEstadisticasArea() {
    // TODO: Implementar cálculo de estadísticas en área seleccionada
}

void MainWindow::exportarDatosAnalisis() {
    // TODO: Implementar exportación a CSV/Excel
}

void MainWindow::exportarReportePDF() {
    // TODO: Implementar generación de informe PDF
}

void MainWindow::aplicarFiltroUmbral(int valor) {
    Q_UNUSED(valor);
    // TODO: Implementar filtro de umbral de temperatura
}
