#include "clickthroughttextedit.h"
#include <QDebug>
#include <QMouseEvent>
#include <QWidget>
#include <QApplication>
#include <QScrollBar>

ClickthroughtTextEdit::ClickthroughtTextEdit(QWidget *parent) :
    QTextEdit(parent)
{
}

void ClickthroughtTextEdit::mousePressEvent(QMouseEvent *e)
{
    QPoint point=e->pos();
    bool hit=false;
    point.ry()+=verticalScrollBar()->value();

    for (QWidget* obj : clickeeObjects)
    {
        QPoint objpos = obj->pos();
        QRect rect = obj->rect();
        rect.moveTopLeft(objpos);
        if (rect.contains(point))
        {
            QPoint localpoint = point-rect.topLeft();
            //qDebug() << QString("ClickthroughtTextEdit: object clicked at %1,%2")
            //            .arg(localpoint.x()).arg(localpoint.y());
            QMouseEvent newEvent(QEvent::MouseButtonPress, localpoint,
                                Qt::LeftButton, e->buttons(), Qt::NoModifier);
            QApplication::sendEvent(obj, &newEvent);
            hit=true;
            break;
        }
    }
    if (!hit) QTextEdit::mousePressEvent(e);
}

void ClickthroughtTextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    QPoint point=e->pos();
    bool hit=false;
    point.ry()+=verticalScrollBar()->value();

    for (QWidget* obj : clickeeObjects)
    {
        QPoint objpos = obj->pos();
        QRect rect = obj->rect();
        rect.moveTopLeft(objpos);
        if (rect.contains(point))
        {
            QPoint localpoint = point-rect.topLeft();
            //qDebug() << QString("ClickthroughtTextEdit: object clicked at %1,%2")
            //            .arg(localpoint.x()).arg(localpoint.y());
            QMouseEvent newEvent(QEvent::MouseButtonRelease, localpoint,
                                Qt::LeftButton, e->buttons(), Qt::NoModifier);
            QApplication::sendEvent(obj, &newEvent);
            hit=true;
            break;
        }
    }
    if (!hit) QTextEdit::mousePressEvent(e);
}

void ClickthroughtTextEdit::addClickee(QWidget* obj)
{
    clickeeObjects.append(obj);

    connect(obj, SIGNAL(needsRedrawing()), this, SLOT(update()));
}
