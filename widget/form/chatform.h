#ifndef CHATFORM_H
#define CHATFORM_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTime>
#include <QPoint>

#include "core.h"
#include "widget/form/abstractchatform.h"

struct Friend;
class NetCamView;
class QLabel;
class QPushButton;

class ChatForm : public AbstractChatForm
{
    Q_OBJECT
public:
    ChatForm(Friend* chatFriend);
    ~ChatForm();
    void setName(QString newName);
    void setStatusMessage(QString newMessage);
    void addFriendMessage(QString message);

signals:
    void sendMessage(int, QString);
    void sendFile(int32_t friendId, QString, QByteArray);
    void startCall(int friendId);
    void startVideoCall(int friendId, bool video);
    void answerCall(int callId);
    void hangupCall(int callId);
    void cancelCall(int callId, int friendId);

public slots:
    void startFileSend(ToxFile file);
    void onFileRecvRequest(ToxFile file);
    void onAvInvite(int FriendId, int CallId, bool video);
    void onAvStart(int FriendId, int CallId, bool video);
    void onAvCancel(int FriendId, int CallId);
    void onAvEnd(int FriendId, int CallId);
    void onAvRinging(int FriendId, int CallId, bool video);
    void onAvStarting(int FriendId, int CallId, bool video);
    void onAvEnding(int FriendId, int CallId);
    void onAvRequestTimeout(int FriendId, int CallId);
    void onAvPeerTimeout(int FriendId, int CallId);

private slots:
    virtual void onSendTriggered() override;
    void onAttachClicked();
    void onCallTriggered();
    void onVideoCallTriggered();
    void onAnswerCallTriggered();
    void onHangupCallTriggered();
    void onCancelCallTriggered();
    void onChatContextMenuRequested(QPoint pos);
    void onSaveLogClicked();

private:
    Friend* f;
    QHBoxLayout *headLayout;
    QVBoxLayout *headTextLayout, *footButtonsSmall;
    QLabel *avatar, *name, *statusMessage;
    QPushButton *sendButton, *fileButton, *emoteButton, *callButton, *videoButton;
    NetCamView* netcam;
    int curRow;
    int callId;
};

#endif // CHATFORM_H
