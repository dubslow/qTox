#ifndef CLICKTHROUGHTTEXTEDIT_H
#define CLICKTHROUGHTTEXTEDIT_H

#include <QTextEdit>

class ClickthroughtTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit ClickthroughtTextEdit(QWidget *parent = 0);

    void addClickee(QWidget* obj);

protected:
    // We need to forward the mouse events to the clickee objects ourselves
    // They can't receive anything by themselves since they believe they are text
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

private:
    // List of widgets currently embedded in the QTextEdit's QTextDocument
    QList<QWidget*> clickeeObjects;
};

#endif // CLICKTHROUGHTTEXTEDIT_H
