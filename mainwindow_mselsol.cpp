#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsPixmapItem>
#include <QAction>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPen>
#include <QApplication>

#include <QInputDialog>      // <<-- para pedir texto
#include <QLineEdit>         // <<-- QLineEdit::Normal
#include <QGraphicsTextItem> // <<-- para dibujar texto
#include <QDebug>            // <<-- para prints de debug
#include <QMessageBox>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))

    , view(new QGraphicsView(this))
{
    ui->setupUi(this);

    setWindowTitle("Carta Náutica");
    view->setScene(scene);
    setCentralWidget(view);

    //cargamos la carta nautica en un PixmapItem
    QPixmap pm(":/resources/carta_nautica.jpg");
    QGraphicsPixmapItem *item = scene->addPixmap(pm);
    item->setZValue(0);

    // escalado en mi portatil para que se ajuste aprox a mi ventana
    view->scale(0.20, 0.20);
    m_scaleFactor = 0.20;

    //DragMode permite mover toda la imagen con el ratón
    view->setDragMode(QGraphicsView::ScrollHandDrag);

    // el EventFilter evita el problema de que los eventos
    // lleguen primero al widget y no al QGraphicsView
    view->viewport()->installEventFilter(this);

    // connect con los QAction, se pude hacer desde designer
    QAction *actZoomIn = ui->toolBar->addAction("+");
    QAction *actZoomOut = ui->toolBar->addAction("-");

    connect(actZoomIn, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(actZoomOut, &QAction::triggered, this, &MainWindow::zoomOut);

    // NUEVO: QAction toggle para dibujar línea
    m_actDrawLine = ui->toolBar->addAction(tr("Línea"));
    m_actDrawLine->setCheckable(true);
    connect(m_actDrawLine, &QAction::toggled, this, &MainWindow::setDrawLineMode);


    // ==============================================================
    // ======= NUEVO: Acción para anotar texto =======
    // =============================================================

    m_actAddText = ui->toolBar->addAction("Texto");
    m_actAddText->setCheckable(true);
    connect(m_actAddText, &QAction::toggled, this, &MainWindow::setTextMode);

    // tamaño texto
    // ---- Tamaño del texto ----
    m_textSizeBox = new QSpinBox(this);
    m_textSizeBox->setRange(8, 250);
    m_textSizeBox->setValue(50);
    ui->toolBar->addWidget(m_textSizeBox);
    m_textSizeBox->setVisible(false); // inicialmente oculto

    // ---- Colores ----
    m_colorBlack = ui->toolBar->addAction("Negro");
    m_colorRed   = ui->toolBar->addAction("Rojo");
    m_colorBlue  = ui->toolBar->addAction("Azul");

    // acciones: seleccionar color
    connect(m_colorBlack, &QAction::triggered, [this]() {
        m_currentTextColor = Qt::black;
          qDebug() << "COLOR -> NEGRO";
    });
    connect(m_colorRed, &QAction::triggered, [this]() {
        m_currentTextColor = Qt::red;
          qDebug() << "COLOR -> RED";
    });
    connect(m_colorBlue, &QAction::triggered, [this]() {
        m_currentTextColor = Qt::blue;
          qDebug() << "COLOR -> BLUE";
    });

    // Inicialmente ocultos
    m_colorBlack->setVisible(false);
    m_colorRed->setVisible(false);
    m_colorBlue->setVisible(false);

    // ==============================================================
    // ======= NUEVO: Acción para borrar marcas =======
    // ==============================================================

    m_actErase = ui->toolBar->addAction("Borrar");
    m_actErase->setCheckable(true);
    connect(m_actErase, &QAction::toggled, this, &MainWindow::setEraseMode);


    // =====================================================
    // === Acción LIMPIAR la carta (tarea 8)
    // =====================================================
    m_actClear = ui->toolBar->addAction("Limpiar");

    connect(m_actClear, &QAction::triggered, this, &MainWindow::clearAllMarks);

    // =====================================================
    // === Acción TRANSPORTADOR (tarea 9)
    // =====================================================
    m_actProtractor = ui->toolBar->addAction("Transportador");
    m_actProtractor->setCheckable(true);
    connect(m_actProtractor, &QAction::toggled, this, &MainWindow::setProtractorMode);


    //  Regla dentro de la escena
    m_protractor = new Tool(":/resources/icons/transportador.svg");
    scene->addItem(m_protractor);

    // Tamaño "lógico" aproximado, como antes
    m_protractor->setToolSize(QSizeF(1280, 1080));

    // Z alta para que quede en overlay
    m_protractor->setZValue(1000);

    // Posición inicial en la esquina superior izquierda de la vista
    m_protractor->setPos(view->mapToScene(20, 20));
    m_protractor->setVisible(false);  // no visible al inicio

}

void MainWindow::zoomIn()
{
    applyZoom(1.15);
}

void MainWindow::zoomOut()
{
    applyZoom(1.0 / 1.15);
}


void MainWindow::applyZoom(double factor)
{
    // factor > 1 => acercar; factor < 1 => alejar
    double newScale = m_scaleFactor * factor;
    const double minScale = 0.01;
    const double maxScale = 1;

    if (newScale < minScale) {
        factor = minScale / m_scaleFactor;
        newScale = minScale;
    } else if (newScale > maxScale) {
        factor = maxScale / m_scaleFactor;
        newScale = maxScale;
    }

    view->scale(factor, factor);
    m_scaleFactor = newScale;
}

void MainWindow::setDrawLineMode(bool enabled)
{
    m_drawLineMode = enabled;

    if (enabled) {
        view->setCursor(Qt::CrossCursor);

        // Apagar modo texto
        if (m_actAddText && m_actAddText->isChecked()) {
            m_actAddText->setChecked(false);
            m_textMode = false;
        }

        // Apagar modo borrar
        if (m_actErase && m_actErase->isChecked()) {
            m_actErase->setChecked(false);
            m_eraseMode = false;
        }

        // Ocultar controles de texto
        m_textSizeBox->setVisible(false);
        m_colorBlack->setVisible(false);
        m_colorRed->setVisible(false);
        m_colorBlue->setVisible(false);

        statusBar()->showMessage("Modo línea activado");
    } else {
        view->unsetCursor();
        statusBar()->clearMessage();
    }
}

// ==============================================================
// ======= NUEVO: Acción para añadir texto  6 ==================
// ==============================================================


void MainWindow::setTextMode(bool enabled)
{
    m_textMode = enabled;

    if (enabled) {
        view->setCursor(Qt::IBeamCursor);

        // Apagar modo línea
        if (m_actDrawLine && m_actDrawLine->isChecked()) {
            m_actDrawLine->setChecked(false);
            m_drawLineMode = false;
        }

        // Apagar modo borrar
        if (m_actErase && m_actErase->isChecked()) {
            m_actErase->setChecked(false);
            m_eraseMode = false;
        }

        // Mostrar controles de texto
        m_textSizeBox->setVisible(true);
        m_colorBlack->setVisible(true);
        m_colorRed->setVisible(true);
        m_colorBlue->setVisible(true);

        statusBar()->showMessage("Modo texto activado");
    } else {
        view->unsetCursor();

        // Puedes decidir si ocultar o no los controles de texto aquí
        m_textSizeBox->setVisible(false);
        m_colorBlack->setVisible(false);
        m_colorRed->setVisible(false);
        m_colorBlue->setVisible(false);

        statusBar()->clearMessage();
    }
}

// ==============================================================
// ======= NUEVO: 7. Acción para borrar marcas =================
// ==============================================================

void MainWindow::setEraseMode(bool enabled)
{
    m_eraseMode = enabled;

    if (enabled) {
        view->setCursor(Qt::PointingHandCursor);

        // Apagar modo línea
        if (m_actDrawLine && m_actDrawLine->isChecked()) {
            m_actDrawLine->setChecked(false);
            m_drawLineMode = false;
        }

        // Apagar modo texto
        if (m_actAddText && m_actAddText->isChecked()) {
            m_actAddText->setChecked(false);
            m_textMode = false;
        }

        // Ocultar controles de texto
        m_textSizeBox->setVisible(false);
        m_colorBlack->setVisible(false);
        m_colorRed->setVisible(false);
        m_colorBlue->setVisible(false);

        statusBar()->showMessage("Modo borrar activado");
    } else {
        view->unsetCursor();
        statusBar()->clearMessage();
    }
}

// ==============================================================
// ======= NUEVO: 8. Acción para resetear y limpiar la carta ===
// ==============================================================

void MainWindow::clearAllMarks()
{
    // Confirmación
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "Limpiar carta",
        "¿Seguro que quieres borrar todas las marcas?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes)
        return;

    // Recorremos los items de la escena
    QList<QGraphicsItem*> all = scene->items();

    for (QGraphicsItem *item : all) {

        // No borrar la carta (z = 0)
        if (item->zValue() <= 0)
            continue;

        // No borrar el transportador
        if (item == m_protractor)
            continue;

        // Borrar la marca
        scene->removeItem(item);
        delete item;
    }
}

void MainWindow::setProtractorMode(bool enabled)
{
    m_protractorMode = enabled;

    if (enabled)
    {
        // Mostrar transportador
        m_protractor->setVisible(true);

        // Desactivar otros modos
        if (m_actDrawLine && m_actDrawLine->isChecked()) {
            m_actDrawLine->setChecked(false);
            m_drawLineMode = false;
        }

        if (m_actAddText && m_actAddText->isChecked()) {
            m_actAddText->setChecked(false);
            m_textMode = false;
            m_textSizeBox->setVisible(false);
            m_colorBlack->setVisible(false);
            m_colorRed->setVisible(false);
            m_colorBlue->setVisible(false);
        }

        if (m_actErase && m_actErase->isChecked()) {
            m_actErase->setChecked(false);
            m_eraseMode = false;
        }

        view->setCursor(Qt::OpenHandCursor);

        statusBar()->showMessage("Modo transportador activado");
    }
    else
    {
        view->unsetCursor();
        statusBar()->clearMessage();
        // ocultarlo al desmarcar la opción.
         m_protractor->setVisible(false);
    }
}




bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == view->viewport()) {

        // Si no hay ningún modo activo, devolvemos false para que la vista gestione normalmente.
        if (!m_drawLineMode && !m_textMode && !m_eraseMode && !m_protractorMode)
            return false;

        // =========================
        // MODO LINEA
        // =========================
        if (m_drawLineMode) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *e = static_cast<QMouseEvent*>(event);
                // uso botón derecho para crear la línea (como tenías)
                if (e->button() == Qt::RightButton) {
                    QPointF scenePos = view->mapToScene(e->pos());
                    m_lineStart = scenePos;

                    QPen pen(Qt::red, 8);
                    m_currentLineItem = new QGraphicsLineItem();
                    m_currentLineItem->setZValue(10);
                    m_currentLineItem->setPen(pen);
                    m_currentLineItem->setLine(QLineF(m_lineStart, m_lineStart));
                    scene->addItem(m_currentLineItem);

                    return true;
                }
            }
            else if (event->type() == QEvent::MouseMove) {
                auto *e = static_cast<QMouseEvent*>(event);
                if (e->buttons() & Qt::RightButton && m_currentLineItem) {
                    QPointF p2 = view->mapToScene(e->pos());
                    m_currentLineItem->setLine(QLineF(m_lineStart, p2));
                    return true;
                }
            }
            else if (event->type() == QEvent::MouseButtonRelease) {
                auto *e = static_cast<QMouseEvent*>(event);
                if (e->button() == Qt::RightButton && m_currentLineItem) {
                    m_currentLineItem = nullptr;
                    return true;
                }
            }

            // si ya atendimos el evento en modo línea, devolvemos true (o lo dejamos pasar si no)
            return false;
        }

        // =========================
        // 7. MODO BORRAR
        // =========================
        if (m_eraseMode) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto e = static_cast<QMouseEvent*>(event);

                if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton) {
                    QPointF scenePos = view->mapToScene(e->pos());

                    // OJO: lista de punteros
                    QList<QGraphicsItem*> itemsAtPos  = scene->items(scenePos);

                    for (QGraphicsItem* item : itemsAtPos) {
                        // No borrar la carta de fondo (z = 0 o menos)
                        if (item->zValue() <= 0)
                            continue;

                        // No borrar el transportador
                        if (item == m_protractor)
                            continue;

                        // Si llega aquí, asumimos que es una marca (línea, texto, etc.)
                        scene->removeItem(item);
                        delete item;
                        break; // borramos solo la primera marca encontrada
                    }

                    return true; // consumimos el evento
                }
            }

            // Otros eventos en modo borrar los dejamos pasar
            return false;
        }
        // =========================
        // 6. MODO TEXTO
        // =========================
        if (m_textMode) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *e = static_cast<QMouseEvent*>(event);
                // Aceptamos clic izquierdo o derecho para mayor usabilidad
                if (e->button() == Qt::RightButton || e->button() == Qt::LeftButton) {
                    QPointF scenePos = view->mapToScene(e->pos());

                    bool ok;
                    QString text = QInputDialog::getText(
                        this,
                        tr("Anotar texto"),
                        tr("Introduce el texto:"),
                        QLineEdit::Normal,
                        QString(),
                        &ok
                        );

                    if (ok && !text.isEmpty()) {
                        QGraphicsTextItem *t = scene->addText(text);

                        QFont font = t->font();
                        font.setPointSize(m_textSizeBox->value()); // << usa tamaño elegido
                        t->setFont(font);

                        t->setDefaultTextColor(m_currentTextColor);
                        t->setZValue(50);
                        t->setPos(scenePos);
                    } else {
                        qDebug() << "Texto cancelado o vacío";
                    }

                    // desactivar modo texto
                    if (m_actAddText)
                        m_actAddText->setChecked(false);
                    m_textMode = false;
                    view->unsetCursor();

                    return true; // consumimos el evento
                }
            }


            // Para movimientos u otros eventos en modo texto no hacemos nada especial
            return false;
        }

        // =====================================================
        // === MODO TRANSPORTADOR: medir ángulos
        // =====================================================
        if (m_protractorMode)
        {
            if (event->type() == QEvent::MouseButtonPress)
            {
                auto *e = static_cast<QMouseEvent*>(event);

                if (e->button() == Qt::LeftButton)
                {
                    // Posición del clic en coordenadas de la escena
                    QPointF scenePos = view->mapToScene(e->pos());

                    // 1. Comprobar si clicas sobre el transportador
                    QPointF local = m_protractor->mapFromScene(scenePos);
                    bool inside = m_protractor->boundingRect().contains(local);

                    if (inside) {
                        // Dejar que el transportador se mueva libremente
                        return false;
                    }

                    // 2. Medir ángulo respecto al centro real del transportador
                    QPointF centerLocal = m_protractor->boundingRect().center();
                    QPointF centerScene = m_protractor->sceneTransform().map(centerLocal);
                          // centro mapeado a escena

                    // Vector desde el centro al clic
                    QPointF d = scenePos - centerScene;

                    // Calcular ángulo
                    double angleRad = std::atan2(-d.y(), d.x());  // Y invertida porque Qt tiene origen arriba
                    double angleDeg = angleRad * 180.0 / M_PI;
                    if (angleDeg < 0) angleDeg += 360.0;

                    // Dibujar línea desde el centro
                    QPen pen(Qt::black, 4);
                    QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(centerScene, scenePos));
                    line->setPen(pen);
                    line->setZValue(500);
                    scene->addItem(line);

                    // Texto con el ángulo
                    QGraphicsTextItem *t = scene->addText(QString::number(angleDeg, 'f', 1) + "°");
                    QFont f;
                    f.setPointSize(28);
                    f.setBold(true);
                    t->setFont(f);
                    t->setDefaultTextColor(Qt::black);
                    t->setZValue(500);
                    t->setPos(scenePos + QPointF(15, -20));

                    return true;
                }
            }

            // Dejar mover la carta o usar rueda libremente
            return false;
        }




    }

    return QMainWindow::eventFilter(obj, event);
}


MainWindow::~MainWindow()
{
    delete ui;
}
