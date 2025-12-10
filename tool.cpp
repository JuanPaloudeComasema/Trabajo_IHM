#include "tool.h"

#include <QtMath>
#include <QApplication>
#include <QGraphicsScene>
#include <QSvgRenderer>

Tool::Tool(const QString& svgResourcePath, QGraphicsItem* parent)
    : QGraphicsSvgItem(svgResourcePath, parent)
{
    // Flags para poder moverla, seleccionarla y que ignore las transformaciones del view
        setFlags(QGraphicsItem::ItemIsMovable
                 | QGraphicsItem::ItemIsSelectable
                 | QGraphicsItem::ItemSendsGeometryChanges);


    // Origen de rotación = centro del SVG
    updateOrigin();

    // Tamaño inicial = tamaño natural del SVG
    m_targetSizePx = boundingRect().size();
    applyInitialScale();
}

void Tool::setToolSize(const QSizeF& sizePx)
{
    m_targetSizePx = sizePx;
    applyInitialScale();
}

void Tool::applyInitialScale()
{
    const QRectF br = boundingRect();
    if (br.isEmpty())
        return;

    const double sx = m_targetSizePx.width()  / br.width();
    const double sy = m_targetSizePx.height() / br.height();
    m_uniformScale = std::min(sx, sy);

    setScale(m_uniformScale);
    updateOrigin();

    if (scene())
        scene()->update();
}

void Tool::updateOrigin()
{
    setTransformOriginPoint(boundingRect().center());
}

void Tool::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    // Solo rotar si está pulsado Shift
    if (!(QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
        event->ignore();
        return;
    }

    const int delta = event->delta(); // Qt5: 120 por "clic" de rueda
    if (delta == 0) {
        event->ignore();
        return;
    }

    // Igual que antes: rotación suave
    double deltaDegrees = (delta / 8.0) * 0.1; // ≈ 1.5° por "clic"
    m_angleDeg += deltaDegrees;
    setRotation(m_angleDeg);

    if (scene())
        scene()->update();

    event->accept();
}

void Tool::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_lastScenePos = event->scenePos();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
}

void Tool::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF deltaScene = event->scenePos() - m_lastScenePos;

    if (event->modifiers() & Qt::ShiftModifier) {
        // Rotación basada en posiciones de escena
        double angle = std::atan2(deltaScene.y(), deltaScene.x()) * 180.0 / M_PI;
        setRotation(rotation() + angle / 10.0);
    }
    else {
        // Movimiento suave en escena
        setPos(pos() + deltaScene);
    }

    m_lastScenePos = event->scenePos();
    event->accept();
}


void Tool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::OpenHandCursor);
    event->accept();
}

