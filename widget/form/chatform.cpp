#include "chatform.h"
#include "friend.h"
#include "widget/friendwidget.h"
#include "widget/widget.h"
#include "widget/filetransfertwidget.h"
#include "widget/netcamview.h"
#include "widget/tool/chattextedit.h"
#include "widget/tool/clickthroughttextedit.h"
#include <QFont>
#include <QTime>
#include <QScrollBar>
#include <QFileDialog>
#include <QMenu>
#include <QTextTable>
#include <QVariant>

ChatForm::ChatForm(Friend* chatFriend)
    : AbstractChatForm(), f(chatFriend), curRow{0}
{
    name = new QLabel(), avatar = new QLabel(), statusMessage = new QLabel();
    headLayout = new QHBoxLayout();
    headTextLayout = new QVBoxLayout(), footButtonsSmall = new QVBoxLayout();
    sendButton = new QPushButton(), fileButton = new QPushButton(), emoteButton = new QPushButton();
    callButton = new QPushButton(), videoButton = new QPushButton();
    netcam = new NetCamView();

    QFont bold;
    bold.setBold(true);
    name->setText(chatFriend->widget->name.text());
    name->setFont(bold);
    statusMessage->setText(chatFriend->widget->statusMessage.text());

    // No real avatar support in toxcore, better draw a pretty picture
    //avatar->setPixmap(*chatFriend->widget->avatar.pixmap());
    avatar->setPixmap(QPixmap(":/img/contact_dark.png"));

    footButtonsSmall->setSpacing(2);

    QString sendButtonStylesheet = "";
    try
    {
        QFile f(":/ui/sendButton/sendButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream sendButtonStylesheetStream(&f);
        sendButtonStylesheet = sendButtonStylesheetStream.readAll();
    }
    catch (int e) {}
    sendButton->setStyleSheet(sendButtonStylesheet);

    QString fileButtonStylesheet = "";
    try
    {
        QFile f(":/ui/fileButton/fileButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream fileButtonStylesheetStream(&f);
        fileButtonStylesheet = fileButtonStylesheetStream.readAll();
    }
    catch (int e) {}
    fileButton->setStyleSheet(fileButtonStylesheet);


    QString emoteButtonStylesheet = "";
    try
    {
        QFile f(":/ui/emoteButton/emoteButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream emoteButtonStylesheetStream(&f);
        emoteButtonStylesheet = emoteButtonStylesheetStream.readAll();
    }
    catch (int e) {}
    emoteButton->setStyleSheet(emoteButtonStylesheet);

    QString callButtonStylesheet = "";
    try
    {
        QFile f(":/ui/callButton/callButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream callButtonStylesheetStream(&f);
        callButtonStylesheet = callButtonStylesheetStream.readAll();
    }
    catch (int e) {}
    callButton->setObjectName("green");
    callButton->setStyleSheet(callButtonStylesheet);

    QString videoButtonStylesheet = "";
    try
    {
        QFile f(":/ui/videoButton/videoButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream videoButtonStylesheetStream(&f);
        videoButtonStylesheet = videoButtonStylesheetStream.readAll();
    }
    catch (int e) {}
    videoButton->setObjectName("green");
    videoButton->setStyleSheet(videoButtonStylesheet);

    mainLayout->addWidget(chatArea);
    mainLayout->addLayout(mainFootLayout);
    mainLayout->setMargin(0);

    footButtonsSmall->addWidget(emoteButton);
    footButtonsSmall->addWidget(fileButton);

    mainFootLayout->addLayout(footButtonsSmall);
    mainFootLayout->addSpacing(5);
    mainFootLayout->addWidget(sendButton);
    mainFootLayout->setSpacing(0);

    head->setLayout(headLayout);
    headLayout->addWidget(avatar);
    headLayout->addLayout(headTextLayout);
    headLayout->addStretch();
    headLayout->addWidget(callButton);
    headLayout->addWidget(videoButton);

    headTextLayout->addStretch();
    headTextLayout->addWidget(name);
    headTextLayout->addWidget(statusMessage);
    headTextLayout->addStretch();

    chatArea->setWidget(chatAreaWidget);

    //Fix for incorrect layouts on OS X as per
    //https://bugreports.qt-project.org/browse/QTBUG-14591
    sendButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    fileButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    emoteButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    callButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    videoButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    connect(Widget::getInstance()->getCore(), &Core::fileSendStarted, this, &ChatForm::startFileSend);
    connect(Widget::getInstance()->getCore(), &Core::videoFrameReceived, netcam, &NetCamView::updateDisplay);
    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(fileButton, SIGNAL(clicked()), this, SLOT(onAttachClicked()));
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
}

ChatForm::~ChatForm()
{
    delete main;
    delete head;
    delete netcam;
}

void ChatForm::setName(QString newName)
{
    name->setText(newName);
}

void ChatForm::setStatusMessage(QString newMessage)
{
    statusMessage->setText(newMessage);
}

void ChatForm::addFriendMessage(QString message)
{
    addMessage(name->text(), message);
}

void ChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;
    QString name = Widget::getInstance()->getUsername();
    msgEdit->clear();
    addMessage(name, msg);
    emit sendMessage(f->friendId, msg);
}

void ChatForm::onAttachClicked()
{
    QString path = QFileDialog::getOpenFileName(0,tr("Send a file"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return;
    QByteArray fileData = file.readAll();
    file.close();
    QFileInfo fi(path);

    emit sendFile(f->friendId, fi.fileName(), fileData);
}

void ChatForm::startFileSend(ToxFile file)
{
    if (file.friendId != f->friendId)
        return;
    QTextBlockFormat rightAlign;
    rightAlign.setAlignment(Qt::AlignRight);
    rightAlign.setNonBreakableLines(true);
    QTextBlockFormat leftAlign;
    leftAlign.setAlignment(Qt::AlignLeft);
    leftAlign.setNonBreakableLines(true);
    int row=chatTable->rows()-1;
    QLabel *author = new QLabel(Widget::getInstance()->getUsername());
    QLabel *date = new QLabel(QTime::currentTime().toString("hh:mm"));
    QScrollBar* scroll = chatArea->verticalScrollBar();
    lockSliderToBottom = scroll && scroll->value() == scroll->maximum();
    author->setAlignment(Qt::AlignTop | Qt::AlignRight);
    date->setAlignment(Qt::AlignTop);
    QPalette pal;
    pal.setColor(QPalette::WindowText, Qt::gray);
    author->setPalette(pal);
    if (previousName.isEmpty() || previousName != author->text())
    {
        if (curRow)
        {
            chatTable->appendRows(1);
            curRow++;
        }
        chatTable->cellAt(row,0).firstCursorPosition().setBlockFormat(rightAlign);
        chatTable->cellAt(row,0).firstCursorPosition().insertText(author->text());
    }
    previousName = author->text();
    curRow++;

    QTextCharFormat charFormat;
    QVariant storedFile;
    storedFile.setValue(file);
    fileTransfertTextFormat++;
    QWidget *fileTransferInterface = new FileTransfertWidget();
    chatAreaWidget->addClickee(fileTransferInterface);
    chatAreaWidget->document()->documentLayout()->registerHandler(fileTransfertTextFormat, fileTransferInterface);
    charFormat.setObjectType(fileTransfertTextFormat);
    charFormat.setProperty(fileTransfertTextFormat, storedFile);
    QPoint widgetpos = chatAreaWidget->cursorRect(chatTable->cellAt(row,1).firstCursorPosition()).topLeft();
    fileTransferInterface->move(widgetpos);

    chatTable->cellAt(row,1).firstCursorPosition().insertText(QString(QChar::ObjectReplacementCharacter), charFormat);
    chatTable->cellAt(row,2).firstCursorPosition().setBlockFormat(leftAlign);
    chatTable->cellAt(row,2).firstCursorPosition().insertText(date->text());
    chatTable->appendRows(1);
}

void ChatForm::onFileRecvRequest(ToxFile file)
{
    if (file.friendId != f->friendId)
        return;
    QTextBlockFormat rightAlign;
    rightAlign.setAlignment(Qt::AlignRight);
    rightAlign.setNonBreakableLines(true);
    QTextBlockFormat leftAlign;
    leftAlign.setAlignment(Qt::AlignLeft);
    leftAlign.setNonBreakableLines(true);
    int row=chatTable->rows()-1;
    QLabel *author = new QLabel(f->getName());
    QLabel *date = new QLabel(QTime::currentTime().toString("hh:mm"));
    QScrollBar* scroll = chatArea->verticalScrollBar();
    lockSliderToBottom = scroll && scroll->value() == scroll->maximum();
    author->setAlignment(Qt::AlignTop | Qt::AlignRight);
    date->setAlignment(Qt::AlignTop);
    if (previousName.isEmpty() || previousName != author->text())
    {
        if (curRow)
        {
            chatTable->appendRows(1);
            curRow++;
        }
        chatTable->cellAt(row,0).firstCursorPosition().setBlockFormat(rightAlign);
        chatTable->cellAt(row,0).firstCursorPosition().insertText(author->text());
    }
    previousName = author->text();
    curRow++;

    QTextCharFormat charFormat;
    QVariant storedFile;
    storedFile.setValue(file);
    fileTransfertTextFormat++;
    QWidget *fileTransferInterface = new FileTransfertWidget();
    chatAreaWidget->addClickee(fileTransferInterface);
    chatAreaWidget->document()->documentLayout()->registerHandler(fileTransfertTextFormat, fileTransferInterface);
    charFormat.setObjectType(fileTransfertTextFormat);
    charFormat.setProperty(fileTransfertTextFormat, storedFile);
    QPoint widgetpos = chatAreaWidget->cursorRect(chatTable->cellAt(row,1).firstCursorPosition()).topLeft();
    fileTransferInterface->move(widgetpos);

    chatTable->cellAt(row,1).firstCursorPosition().insertText(QString(QChar::ObjectReplacementCharacter), charFormat);
    chatTable->cellAt(row,2).firstCursorPosition().setBlockFormat(leftAlign);
    chatTable->cellAt(row,2).firstCursorPosition().insertText(date->text());
    chatTable->appendRows(1);
}

void ChatForm::onAvInvite(int FriendId, int CallId, bool video)
{
    if (FriendId != f->friendId)
        return;
    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("yellow");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onAnswerCallTriggered()));
    }
    else
    {
        callButton->setObjectName("yellow");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onAnswerCallTriggered()));
    }

    Widget* w = Widget::getInstance();
    if (!w->isFriendWidgetCurActiveWidget(f))
    {
        w->newMessageAlert();
        f->hasNewMessages=true;
        w->updateFriendStatusLights(f->friendId);
    }
}

void ChatForm::onAvStart(int FriendId, int CallId, bool video)
{
    if (FriendId != f->friendId)
        return;
    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("red");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
        netcam->show();
    }
    else
    {
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
    }
}

void ChatForm::onAvCancel(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvEnd(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvRinging(int FriendId, int CallId, bool video)
{
    if (FriendId != f->friendId)
        return;
    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("yellow");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onCancelCallTriggered()));
    }
    else
    {
        callButton->setObjectName("yellow");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onCancelCallTriggered()));
    }
}

void ChatForm::onAvStarting(int FriendId, int, bool video)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("red");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
        netcam->show();
    }
    else
    {
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
    }
}

void ChatForm::onAvEnding(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvRequestTimeout(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvPeerTimeout(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAnswerCallTriggered()
{
    emit answerCall(callId);
}

void ChatForm::onHangupCallTriggered()
{
    emit hangupCall(callId);
}

void ChatForm::onCallTriggered()
{
    callButton->disconnect();
    videoButton->disconnect();
    emit startCall(f->friendId);
}

void ChatForm::onVideoCallTriggered()
{
    callButton->disconnect();
    videoButton->disconnect();
    emit startVideoCall(f->friendId, true);
}

void ChatForm::onCancelCallTriggered()
{
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
    emit cancelCall(callId, f->friendId);
}

void ChatForm::onChatContextMenuRequested(QPoint pos)
{
    QWidget* sender = (QWidget*)QObject::sender();
    pos = sender->mapToGlobal(pos);
    QMenu menu;
    menu.addAction(tr("Save chat log"), this, SLOT(onSaveLogClicked()));
    menu.exec(pos);
}

void ChatForm::onSaveLogClicked()
{
    QString path = QFileDialog::getSaveFileName(0,tr("Save chat log"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QString log;
    QList<QLabel*> labels = chatAreaWidget->findChildren<QLabel*>();
    int i=0;
    for (QLabel* label : labels)
    {
        log += label->text();
        if (i==2)
        {
            i=0;
            log += '\n';
        }
        else
        {
            log += '\t';
            i++;
        }
    }

    file.write(log.toUtf8());
    file.close();
}
