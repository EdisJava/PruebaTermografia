#include "mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_manager(new ImagePointManager(this))
{
    setupUI();
    createMenuBar();

    // Conectar señales
    connect(m_imageWidget, &ImageWidget::rightClicked,
            m_manager, &ImagePointManager::addPixelToCurrentHotspot);
    connect(m_manager, SIGNAL(hotspotUpdated(ImagePoint)),
            this, SLOT(onHotspotUpdated(ImagePoint)));
    connect(m_manager, &ImagePointManager::hotspotsCleared,
            this, &MainWindow::onHotspotsCleared);
    connect(m_manager, &ImagePointManager::hotspotColorChanged,
            this, &MainWindow::updatePixelMarkers);
    connect(m_imageWidget, &ImageWidget::colorSelected,
            m_manager, &ImagePointManager::setCurrentHotspotColor);
    connect(m_manager, &ImagePointManager::hotspotColorChanged,
            this, &MainWindow::updatePixelMarkers);
    connect(m_imageWidget, &ImageWidget::colorSelected,
            m_manager, &ImagePointManager::setCurrentHotspotColor);


}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("Image Point Picker - Hotspots");
    resize(900, 600);

    QWidget* centralWidget = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    // Panel izquierdo: imagen
    m_imageWidget = new ImageWidget(this);
    mainLayout->addWidget(m_imageWidget, 2);

    // Panel derecho: lista de hotspots
    QWidget* rightPanel = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    QLabel* listLabel = new QLabel("Hotspots (Click derecho para añadir):", this);
    rightLayout->addWidget(listLabel);

    m_hotspotsList = new QListWidget(this);
    rightLayout->addWidget(m_hotspotsList);

    QPushButton* clearButton = new QPushButton("Limpiar todo", this);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearHotspots);
    rightLayout->addWidget(clearButton);

    mainLayout->addWidget(rightPanel, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::createMenuBar()
{
    QMenu* fileMenu = menuBar()->addMenu("Archivo");

    QAction* openAction = new QAction("Abrir imagen", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    fileMenu->addAction(openAction);

    fileMenu->addSeparator();

    QAction* exitAction = new QAction("Salir", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(exitAction);
}

void MainWindow::openImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Seleccionar imagen",
        QString(),
        "Imágenes (*.png *.jpg *.jpeg *.bmp *.gif)"
        );

    if (fileName.isEmpty()) {
        return;
    }

    QImage image(fileName);
    if (image.isNull()) {
        QMessageBox::warning(this, "Error", "No se pudo cargar la imagen");
        return;
    }

    m_manager->setImage(image);
    m_imageWidget->setImage(image);
}

void MainWindow::clearHotspots()
{
    m_manager->clearHotspots();
}

void MainWindow::onHotspotUpdated(const ImagePoint& hotspot)
{
    qDebug() << "onHotspotUpdated called. Hotspot ID:" << hotspot.id() << "Pixels:" << hotspot.pixelCount();
    Q_UNUSED(hotspot);
    updateHotspotsList();
    updatePixelMarkers();
}

void MainWindow::onHotspotsCleared()
{
    m_hotspotsList->clear();
    m_imageWidget->clearPixelMarkers();
}

void MainWindow::updateHotspotsList()
{
    m_hotspotsList->clear();

    for (const ImagePoint& hotspot : m_manager->hotspots()) {
        m_hotspotsList->addItem(hotspot.toString());
    }
    qDebug() << "Hotspots in manager:" << m_manager->hotspots().size();

}

void MainWindow::updatePixelMarkers()
{
    QList<QPoint> allPixels;

    for (const ImagePoint& hotspot : m_manager->hotspots()) {
        allPixels.append(hotspot.pixels());
    }

    m_imageWidget->setHotspots(m_manager->hotspots()); // repinta todos con color actualizado

}
