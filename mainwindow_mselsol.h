#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
#include <QGraphicsLineItem>
#include <QAction>
#include <QSpinBox>
#include "tool.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;



private slots:
    void zoomIn();
    void zoomOut();

    void setDrawLineMode(bool enabled);

    void setTextMode(bool enabled);

    void setEraseMode(bool enabled);

    void clearAllMarks();

    void setProtractorMode(bool enabled);



private:
    Ui::MainWindow *ui;

    QGraphicsScene *scene;
    QGraphicsView *view;

    double m_scaleFactor = 1.0;

    Tool* m_protractor;       // ahora es un QGraphicsSvgItem dentro de la escena

    QAction *m_actDrawLine = nullptr;
    bool m_drawLineMode = false;
    QGraphicsLineItem *m_tempLine = nullptr;
    QGraphicsLineItem *m_currentLineItem = nullptr;
    QPointF m_lineStart;

    void applyZoom(double factor);

    // Texto
    QAction *m_actAddText = nullptr;
    bool m_textMode = false;
    QSpinBox *m_textSizeBox = nullptr;

    QAction *m_colorBlack = nullptr;
    QAction *m_colorRed = nullptr;
    QAction *m_colorBlue = nullptr;

    QColor m_currentTextColor = Qt::black;

    // Modo borrador
    bool m_eraseMode = false;
    QAction *m_actErase = nullptr;

    // Modo Reset, Clear carta
    QAction *m_actClear = nullptr;

    // Botón Transportador
    QAction *m_actProtractor = nullptr;
    bool m_protractorMode = false;


};

#endif // MAINWINDOW_H
