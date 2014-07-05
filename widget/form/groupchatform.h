#ifndef GROUPCHATFORM_H
#define GROUPCHATFORM_H

#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextEdit>
#include <QScrollArea>
#include <QTime>

#include "widget/form/abstractchatform.h"

class Group;
class QPushButton;

class GroupChatForm : public AbstractChatForm
{
    Q_OBJECT
public:
    GroupChatForm(Group* chatGroup);
    ~GroupChatForm();
    void setName(QString newName);
    void addGroupMessage(QString message, int peerId);
    void addMessage(QString author, QString message, QString date=QTime::currentTime().toString("hh:mm"));
    void addMessage(QLabel* author, QLabel* message, QLabel* date);
    void onUserListChanged();

signals:
    void sendMessage(int, QString);

private slots:
    void onSendTriggered();
    void onSliderRangeChanged();
    void onChatContextMenuRequested(QPoint pos);
    void onSaveLogClicked();

private:
    Group* group;
    QHBoxLayout *headLayout;
    QVBoxLayout *headTextLayout;
    QGridLayout *mainChatLayout;
    QLabel *avatar, *name, *nusers, *namesList;
    QPushButton *sendButton;
    QString previousName;
    int curRow;
    bool lockSliderToBottom;
};

#endif // GROUPCHATFORM_H
