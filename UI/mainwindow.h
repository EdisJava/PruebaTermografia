#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QTimer>
#include "termografiamanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_actionImportar_termogramas_triggered();
    void seleccionarTermograma(int index);
    void on_btnVerLista_clicked();

private:
    Ui::MainWindow *ui;
    TermografiaManager *m_manager;
    QPixmap m_currentPixmap;
    QTimer *m_timerEstado;

    // Parámetros de visualización
    double m_opacidadTermica;
    float m_umbralAlarma;
    bool m_mostrarMaximoAuto;

    // Coordenadas del punto máximo (para tooltip)
    int m_puntoMaximoX;
    int m_puntoMaximoY;
    float m_puntoMaximoTemp;

    // Métodos de renderizado
    void dibujarCapaTermografica();
    void pintarLeyendaColores();

    // Métodos de interacción
    QPoint getCoordenadaImagen(QPoint posMouse);
    void mostrarInfoPunto(QPoint pos);
    void marcarPuntoManual(QPoint pos);
    void eliminarPuntoCercano(QPoint pos);

    // Métodos auxiliares
    void actualizarRelojEstado();
    void calcularEstadisticasArea();
    void exportarDatosAnalisis();
    void exportarReportePDF();
    void aplicarFiltroUmbral(int valor);
};

#endif // MAINWINDOW_H
