#ifndef ABSTRACTCHATFORM_H
#define ABSTRACTCHATFORM_H

#include <QObject>
#include <QTime>
#include <QString>

// Spacing in px inserted when the author of the last message changes
#define AUTHOR_CHANGE_SPACING 5

class ClickthroughtTextEdit;
class ChatTextEdit;
class QTextTable;
class QScrollArea;
class QHBoxLayout;
class QVBoxLayout;
class QWidget;
namespace Ui{class Widget;}

class AbstractChatForm : public QObject
{
    Q_OBJECT
public:
    AbstractChatForm();
    void addMessage(QString author, QString message, QString date=QTime::currentTime().toString("hh:mm"));
    void show(Ui::Widget& ui);

private slots:
    virtual void onSendTriggered()=0;
    void onSliderRangeChanged();

public:
    static int fileTransfertTextFormat;

protected:
    ClickthroughtTextEdit* chatAreaWidget;
    QTextTable* chatTable;
    QScrollArea* chatArea;
    ChatTextEdit* msgEdit;
    QWidget *main, *head;

    QHBoxLayout *mainFootLayout;
    QVBoxLayout *mainLayout;

     bool lockSliderToBottom;
     QString previousName;
};

#endif // ABSTRACTCHATFORM_H
