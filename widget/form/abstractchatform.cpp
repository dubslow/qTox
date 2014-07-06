#include "abstractchatform.h"
#include "widget/tool/chattextedit.h"
#include "widget/tool/clickthroughttextedit.h"
#include "ui_widget.h"
#include "widget/widget.h"
#include <QTextFormat>
#include <QTextTable>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QScrollArea>
#include <QScrollBar>

int AbstractChatForm::fileTransfertTextFormat{QTextFormat::UserObject + 1};

AbstractChatForm::AbstractChatForm()
    : lockSliderToBottom{true}
{
    main = new QWidget(), head = new QWidget();
    mainFootLayout = new QHBoxLayout();
    mainLayout = new QVBoxLayout();

    msgEdit = new ChatTextEdit();
    QFile f1(":/ui/msgEdit/msgEdit.css");
    f1.open(QFile::ReadOnly | QFile::Text);
    msgEdit->setStyleSheet(QTextStream(&f1).readAll());
    f1.close();
    msgEdit->setFixedHeight(50);
    msgEdit->setFrameStyle(QFrame::NoFrame);

    chatAreaWidget = new ClickthroughtTextEdit();
    chatAreaWidget->setReadOnly(true);
    chatAreaWidget->viewport()->setCursor(Qt::ArrowCursor);

    chatArea = new QScrollArea();
    QFile f2(":/ui/chatArea/chatArea.css");
    f2.open(QFile::ReadOnly | QFile::Text);
    chatArea->setStyleSheet(QTextStream(&f2).readAll());
    f2.close();
    chatArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    chatArea->setWidgetResizable(true);
    chatArea->setContextMenuPolicy(Qt::CustomContextMenu);
    chatArea->setFrameStyle(QFrame::NoFrame);
    chatArea->setWidget(chatAreaWidget);

    chatAreaWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    QTextTableFormat tableFormat;
    tableFormat.setColumnWidthConstraints({QTextLength(QTextLength::VariableLength,0),
                                          QTextLength(QTextLength::PercentageLength,100),
                                          QTextLength(QTextLength::VariableLength,0)});
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    chatTable = chatAreaWidget->textCursor().insertTable(1,3);
    chatTable->setFormat(tableFormat);

    main->setLayout(mainLayout);

    mainFootLayout->addWidget(msgEdit);

    connect(msgEdit, SIGNAL(enterPressed()), this, SLOT(onSendTriggered()));
    connect(chatArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(onSliderRangeChanged()));
    connect(chatAreaWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
}

void AbstractChatForm::show(Ui::Widget &ui)
{
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
}

void AbstractChatForm::addMessage(QString author, QString message, QString date)
{
    int row=chatTable->rows()-1;
    QFont msgFont;
    QTextBlockFormat rightAlign;
    rightAlign.setAlignment(Qt::AlignRight);
    rightAlign.setNonBreakableLines(true);
    QTextBlockFormat leftAlign;
    leftAlign.setAlignment(Qt::AlignLeft);
    leftAlign.setNonBreakableLines(true);
    QScrollBar* scroll = chatArea->verticalScrollBar();
    lockSliderToBottom = scroll && scroll->value() == scroll->maximum();
    /** TODO: Write in gray
    if (author == Widget::getInstance()->getUsername())
    {
        QPalette pal;
        pal.setColor(QPalette::WindowText, QColor(100,100,100));
        author->setPalette(pal);
        message->setPalette(pal);
    }
    */
    if (previousName.isEmpty() || previousName != author)
    {
        if (row)
        {
            chatTable->appendRows(1);
            row++;
        }
        previousName = author;
    }
    else if (row)
        author.clear();
    if (message[0] == '>')
    {
        QTextCharFormat greentext = chatTable->cellAt(row,1).format();
        QPen gpen(QColor(61,204,61));
        gpen.setJoinStyle(Qt::MiterJoin);
        gpen.setCosmetic(false);
        greentext.setTextOutline(gpen);
        chatTable->cellAt(row,1).setFormat(greentext);
    }
    chatTable->cellAt(row,0).firstCursorPosition().setBlockFormat(rightAlign);
    chatTable->cellAt(row,2).firstCursorPosition().setBlockFormat(leftAlign);
    chatTable->cellAt(row,0).firstCursorPosition().insertText(author);
    chatTable->cellAt(row,1).firstCursorPosition().insertText(message);
    chatTable->cellAt(row,2).firstCursorPosition().insertText(date);
    chatTable->appendRows(1);
}

void AbstractChatForm::onSliderRangeChanged()
{
    QScrollBar* scroll = chatArea->verticalScrollBar();
    if (lockSliderToBottom)
         scroll->setValue(scroll->maximum());
}

void AbstractChatForm::onChatContextMenuRequested(QPoint pos)
{
    QWidget* sender = (QWidget*)QObject::sender();
    pos = sender->mapToGlobal(pos);
    QMenu menu;
    menu.addAction(tr("Save chat log"), this, SLOT(onSaveLogClicked()));
    menu.exec(pos);
}

void AbstractChatForm::onSaveLogClicked()
{
    QString path = QFileDialog::getSaveFileName(0,tr("Save chat log"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QString log = chatAreaWidget->toPlainText();

    file.write(log.toUtf8());
    file.close();
}
