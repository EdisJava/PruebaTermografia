#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QPixmap>
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
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // Solo funciones que se conectan a señales
    void on_actionImportar_termogramas_triggered();
    void seleccionarTermograma(int index);

private:
    Ui::MainWindow *ui;
    TermografiaManager* m_manager;
    QPixmap m_currentPixmap;

    // FUNCIONES INTERNAS
    void mostrarInfoPunto(QPoint pos);
    void dibujarCapaTermografica();
    void pintarLeyendaColores();
};

#endif
