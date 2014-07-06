#include "groupchatform.h"
#include "group.h"
#include "widget/groupwidget.h"
#include "widget/widget.h"
#include "friend.h"
#include "friendlist.h"
#include "widget/tool/chattextedit.h"
#include "widget/tool/clickthroughttextedit.h"
#include <QFont>
#include <QTime>
#include <QScrollBar>
#include <QMenu>
#include <QFile>
#include <QFileDialog>

GroupChatForm::GroupChatForm(Group* chatGroup)
    : group(chatGroup), curRow{0}, lockSliderToBottom{true}
{
    headLayout = new QHBoxLayout();
    headTextLayout = new QVBoxLayout();
    mainChatLayout = new QGridLayout();
    avatar = new QLabel(), name = new QLabel(), nusers = new QLabel(), namesList = new QLabel();
    sendButton = new QPushButton();
    QFont bold;
    bold.setBold(true);
    QFont small;
    small.setPixelSize(10);
    name->setText(group->widget->name.text());
    name->setFont(bold);
    nusers->setFont(small);
    nusers->setText(GroupChatForm::tr("%1 users in chat","Number of users in chat").arg(group->peers.size()));
    avatar->setPixmap(QPixmap(":/img/group.png"));
    QString names;
    for (QString& s : group->peers)
        names.append(s+", ");
    names.chop(2);
    namesList->setText(names);
    namesList->setFont(small);

    chatAreaWidget->setLayout(mainChatLayout);
    QString chatAreaStylesheet = "";
    try
    {
        QFile f(":/ui/chatArea/chatArea.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream chatAreaStylesheetStream(&f);
        chatAreaStylesheet = chatAreaStylesheetStream.readAll();
    }
    catch (int e) {}
    chatArea->setStyleSheet(chatAreaStylesheet);
    chatArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    chatArea->setWidgetResizable(true);
    chatArea->setContextMenuPolicy(Qt::CustomContextMenu);
    chatArea->setFrameStyle(QFrame::NoFrame);

    mainChatLayout->setColumnStretch(1,1);
    mainChatLayout->setSpacing(10);


    QString msgEditStylesheet = "";
    try
    {
        QFile f(":/ui/msgEdit/msgEdit.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream msgEditStylesheetStream(&f);
        msgEditStylesheet = msgEditStylesheetStream.readAll();
    }
    catch (int e) {}
    msgEdit->setObjectName("group");
    msgEdit->setStyleSheet(msgEditStylesheet);
    msgEdit->setFixedHeight(50);
    msgEdit->setFrameStyle(QFrame::NoFrame);

    mainChatLayout->setColumnStretch(1,1);
    mainChatLayout->setHorizontalSpacing(10);

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

    sendButton->setFixedSize(50, 50);

    main->setLayout(mainLayout);
    mainLayout->addWidget(chatArea);
    mainLayout->addLayout(mainFootLayout);
    mainLayout->setMargin(0);

    mainFootLayout->addWidget(msgEdit);
    mainFootLayout->addWidget(sendButton);

    head->setLayout(headLayout);
    headLayout->addWidget(avatar);
    headLayout->addLayout(headTextLayout);
    headLayout->addStretch();
    headLayout->setMargin(0);

    headTextLayout->addStretch();
    headTextLayout->addWidget(name);
    headTextLayout->addWidget(nusers);
    headTextLayout->addWidget(namesList);
    headTextLayout->setMargin(0);
    headTextLayout->setSpacing(0);
    headTextLayout->addStretch();

    chatArea->setWidget(chatAreaWidget);

    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(msgEdit, SIGNAL(enterPressed()), this, SLOT(onSendTriggered()));
    connect(chatArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(onSliderRangeChanged()));
    connect(chatArea, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
}

GroupChatForm::~GroupChatForm()
{
    delete head;
    delete main;
}

void GroupChatForm::setName(QString newName)
{
    name->setText(newName);
}

void GroupChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;
    msgEdit->clear();
    emit sendMessage(group->groupId, msg);
}

void GroupChatForm::addGroupMessage(QString message, int peerId)
{
    QString msgAuthor;
    if (group->peers.contains(peerId))
        msgAuthor = group->peers[peerId];
    else
        msgAuthor = tr("<Unknown>");

    addMessage(msgAuthor, message, QTime::currentTime().toString("hh:mm"));
}

void GroupChatForm::onUserListChanged()
{
    nusers->setText(tr("%1 users in chat").arg(group->nPeers));
    QString names;
    for (QString& s : group->peers)
        names.append(s+", ");
    names.chop(2);
    namesList->setText(names);
}
