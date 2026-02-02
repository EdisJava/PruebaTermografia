#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "imagewidget.h"
#include "imagepointmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openImage();
    void clearHotspots();
    void onHotspotUpdated(const ImagePoint& hotspot);
    void onHotspotsCleared();

private:
    void setupUI();
    void createMenuBar();
    void updateHotspotsList();
    void updatePixelMarkers();

    ImageWidget* m_imageWidget;
    QListWidget* m_hotspotsList;
    ImagePointManager* m_manager;
};

#endif // MAINWINDOW_H
