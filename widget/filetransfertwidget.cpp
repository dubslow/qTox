#include "filetransfertwidget.h"
#include "widget.h"
#include "core.h"
#include "math.h"
#include "widget/form/chatform.h"
#include <QFileDialog>
#include <QPixmap>
#include <QPainter>
#include <QTextBlock>
#include <QEnterEvent>

FileTransfertWidget::FileTransfertWidget()
{
    initialized=false;
    // Initialized by intrisicSize()
}

QSizeF FileTransfertWidget::intrinsicSize(QTextDocument*, int, const QTextFormat& format)
{
    if (!initialized)
    {
        initialized=true;
        ToxFile File = format.property(ChatForm::fileTransfertTextFormat).value<ToxFile>();
        lastUpdate = QDateTime::currentDateTime();
        lastBytesSent= 0;
        fileNum = File.fileNum;
        friendId = File.friendId;
        direction = File.direction;

        pic=new QLabel(), filename=new QLabel(), size=new QLabel(), speed=new QLabel(), eta=new QLabel();
        topright = new QPushButton(), bottomright = new QPushButton();
        progress = new QProgressBar();
        mainLayout = new QHBoxLayout(), textLayout = new QHBoxLayout();
        infoLayout = new QVBoxLayout(), buttonLayout = new QVBoxLayout();
        buttonWidget = new QWidget();
        QFont prettysmall;
        prettysmall.setPixelSize(10);
        this->setObjectName("default");
        QFile f0(":/ui/fileTransferWidget/fileTransferWidget.css");
        f0.open(QFile::ReadOnly | QFile::Text);
        QTextStream fileTransfertWidgetStylesheet(&f0);
        this->setStyleSheet(fileTransfertWidgetStylesheet.readAll());
        QPalette greybg;
        greybg.setColor(QPalette::Window, QColor(209,209,209));
        greybg.setColor(QPalette::Base, QColor(150,150,150));
        setPalette(greybg);
        setAutoFillBackground(true);

        setMinimumSize(250,58);
        setMaximumHeight(58);
        setLayout(mainLayout);
        mainLayout->setMargin(0);

        pic->setMaximumHeight(40);
        pic->setContentsMargins(5,0,0,0);
        filename->setText(File.fileName);
        filename->setFont(prettysmall);
        size->setText(getHumanReadableSize(File.filesize));
        size->setFont(prettysmall);
        speed->setText("0B/s");
        speed->setFont(prettysmall);
        eta->setText("00:00");
        eta->setFont(prettysmall);
        progress->setValue(0);
        progress->setMinimumHeight(11);
        progress->setFont(prettysmall);
        progress->setTextVisible(false);
        QPalette whitebg;
        whitebg.setColor(QPalette::Window, QColor(255,255,255));
        buttonWidget->setPalette(whitebg);
        buttonWidget->setAutoFillBackground(true);
        buttonWidget->setLayout(buttonLayout);

        QFile f1(":/ui/stopFileButton/style.css");
        f1.open(QFile::ReadOnly | QFile::Text);
        QTextStream stopFileButtonStylesheetStream(&f1);
        stopFileButtonStylesheet = stopFileButtonStylesheetStream.readAll();

        QFile f2(":/ui/pauseFileButton/style.css");
        f2.open(QFile::ReadOnly | QFile::Text);
        QTextStream pauseFileButtonStylesheetStream(&f2);
        pauseFileButtonStylesheet = pauseFileButtonStylesheetStream.readAll();

        QFile f3(":/ui/acceptFileButton/style.css");
        f3.open(QFile::ReadOnly | QFile::Text);
        QTextStream acceptFileButtonStylesheetStream(&f3);
        acceptFileButtonStylesheet = acceptFileButtonStylesheetStream.readAll();

        topright->setStyleSheet(stopFileButtonStylesheet);
        if (File.direction == ToxFile::SENDING)
        {
            bottomright->setStyleSheet(pauseFileButtonStylesheet);
            connect(topright, SIGNAL(clicked()), this, SLOT(cancelTransfer()));
            connect(bottomright, SIGNAL(clicked()), this, SLOT(pauseResumeSend()));

            QPixmap preview;
            if (preview.loadFromData(File.fileData))
            {
                preview = preview.scaledToHeight(40);
                pic->setPixmap(preview);
            }
        }
        else
        {
            bottomright->setStyleSheet(acceptFileButtonStylesheet);
            connect(topright, SIGNAL(clicked()), this, SLOT(rejectRecvRequest()));
            connect(bottomright, SIGNAL(clicked()), this, SLOT(acceptRecvRequest()));
        }

        QPalette toxgreen;
        toxgreen.setColor(QPalette::Button, QColor(107,194,96)); // Tox Green
        topright->setIconSize(QSize(10,10));
        topright->setMinimumSize(25,28);
        topright->setFlat(true);
        topright->setAutoFillBackground(true);
        topright->setPalette(toxgreen);
        bottomright->setIconSize(QSize(10,10));
        bottomright->setMinimumSize(25,28);
        bottomright->setFlat(true);
        bottomright->setAutoFillBackground(true);
        bottomright->setPalette(toxgreen);

        mainLayout->addStretch();
        mainLayout->addWidget(pic);
        mainLayout->addLayout(infoLayout,3);
        mainLayout->addStretch();
        mainLayout->addWidget(buttonWidget);
        mainLayout->setMargin(0);
        mainLayout->setSpacing(0);

        infoLayout->addWidget(filename);
        infoLayout->addLayout(textLayout);
        infoLayout->addWidget(progress);
        infoLayout->setMargin(4);
        infoLayout->setSpacing(4);

        textLayout->addWidget(size);
        textLayout->addWidget(speed);
        textLayout->addWidget(eta);
        textLayout->setMargin(2);
        textLayout->setSpacing(5);

        buttonLayout->addWidget(topright);
        buttonLayout->addSpacing(2);
        buttonLayout->addWidget(bottomright);
        buttonLayout->setContentsMargins(2,0,0,0);
        buttonLayout->setSpacing(0);

        connect(Widget::getInstance()->getCore(), &Core::fileTransferInfo, this, &FileTransfertWidget::onFileTransferInfo);
        connect(Widget::getInstance()->getCore(), &Core::fileTransferCancelled, this, &FileTransfertWidget::onFileTransferCancelled);
        connect(Widget::getInstance()->getCore(), &Core::fileTransferFinished, this, &FileTransfertWidget::onFileTransferFinished);
    }

    return QSizeF(this->minimumWidth(), this->maximumHeight());
}

void FileTransfertWidget::drawObject(QPainter *painter, const QRectF &rect, QTextDocument*, int, const QTextFormat&)
{
    this->render(painter, QPoint(rect.x(), rect.y()));
}

void FileTransfertWidget::mousePressEvent(QMouseEvent *e)
{
    QPoint localpoint = e->pos();
    if (localpoint == QPoint(0,0)) // Avoid infinite recursion
        return;
    QPoint btnSize = topright->rect().bottomRight() - topright->rect().topLeft();
    QRect toprightRect = QRect(rect().topRight()-QPoint(btnSize.x(),0), rect().topRight()+QPoint(0,btnSize.y()));
    QRect bottomrightRect = QRect(rect().bottomRight()-QPoint(btnSize.x(),0), rect().bottomRight()-QPoint(0,btnSize.y()));

    if (toprightRect.contains(localpoint))
    {
        QMouseEvent newEvent(QEvent::MouseButtonPress, QPoint(0,0),Qt::LeftButton, e->buttons(), Qt::NoModifier);
        QApplication::sendEvent(topright, &newEvent);
    }
    else if (bottomrightRect.contains(localpoint))
    {
        QMouseEvent newEvent(QEvent::MouseButtonPress, QPoint(0,0),Qt::LeftButton, e->buttons(), Qt::NoModifier);
        QApplication::sendEvent(bottomright, &newEvent);
    }
}

void FileTransfertWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QPoint localpoint = e->pos();
    if (localpoint == QPoint(0,0)) // Avoid infinite recursion
        return;
    QPoint btnSize = topright->rect().bottomRight() - topright->rect().topLeft();
    QRect toprightRect = QRect(rect().topRight()-QPoint(btnSize.x(),0), rect().topRight()+QPoint(0,btnSize.y()));
    QRect bottomrightRect = QRect(rect().bottomRight()-QPoint(btnSize.x(),0), rect().bottomRight()-QPoint(0,btnSize.y()));

    if (toprightRect.contains(localpoint))
    {
        QMouseEvent newEvent(QEvent::MouseButtonRelease, QPoint(0,0),Qt::LeftButton, e->buttons(), Qt::NoModifier);
        QApplication::sendEvent(topright, &newEvent);
    }
    else if (bottomrightRect.contains(localpoint))
    {
        QMouseEvent newEvent(QEvent::MouseButtonRelease, QPoint(0,0),Qt::LeftButton, e->buttons(), Qt::NoModifier);
        QApplication::sendEvent(bottomright, &newEvent);
    }
}

void FileTransfertWidget::mouseMoveEvent(QMouseEvent *e)
{
    static bool toprightIn = false, bottomrightIn = false;
    QPoint localpoint = e->pos();
    QPoint btnSize = topright->rect().bottomRight() - topright->rect().topLeft();
    QRect toprightRect = QRect(rect().topRight()-QPoint(btnSize.x(),0), rect().topRight()+QPoint(0,btnSize.y()));
    QRect bottomrightRect = QRect(rect().bottomRight()-QPoint(btnSize.x(),0), rect().bottomRight()-QPoint(0,btnSize.y()));
    QPointF nullpoint(0,0);

    if (toprightRect.contains(localpoint))
    {
        if (!toprightIn)
        {
            toprightIn = true;
            QEnterEvent newEvent(nullpoint, nullpoint, nullpoint);
            QApplication::sendEvent(topright, &newEvent);
        }
    }
    else if (toprightIn)
    {
        toprightIn = false;
        QEvent newEvent(QEvent::Leave);
        QApplication::sendEvent(topright, &newEvent);
    }

    if (bottomrightRect.contains(localpoint))
    {
        if (!bottomrightIn)
        {
            bottomrightIn = true;
            QMouseEvent newEvent(QEvent::MouseMove, QPoint(0,0),Qt::NoButton, e->buttons(), Qt::NoModifier);
            QApplication::sendEvent(bottomright, &newEvent);
        }
    }
    else if (bottomrightIn)
    {
        toprightIn = false;
        QEvent newEvent(QEvent::Leave);
        QApplication::sendEvent(topright, &newEvent);
    }
}

QString FileTransfertWidget::getHumanReadableSize(int size)
{
    static const char* suffix[] = {"B","kiB","MiB","GiB","TiB"};
    int exp = 0;
    if (size)
        exp = std::min( (int) (log(size) / log(1024)), (int) (sizeof(suffix) / sizeof(suffix[0]) - 1));
    return QString().setNum(size / pow(1024, exp),'g',3).append(suffix[exp]);
}

void FileTransfertWidget::onFileTransferInfo(int FriendId, int FileNum, int Filesize, int BytesSent, ToxFile::FileDirection Direction)
{
    if (FileNum != fileNum || FriendId != friendId || Direction != direction)
            return;
    QDateTime newtime = QDateTime::currentDateTime();
    int timediff = lastUpdate.secsTo(newtime);
    if (timediff <= 0)
        return;
    int diff = BytesSent - lastBytesSent;
    if (diff < 0)
        diff = 0;
    int rawspeed = diff / timediff;
    speed->setText(getHumanReadableSize(rawspeed)+"/s");
    size->setText(getHumanReadableSize(Filesize));
    if (!rawspeed)
        return;
    int etaSecs = (Filesize - BytesSent) / rawspeed;
    QTime etaTime(0,0);
    etaTime = etaTime.addSecs(etaSecs);
    eta->setText(etaTime.toString("mm:ss"));
    if (!Filesize)
        progress->setValue(0);
    else
        progress->setValue(BytesSent*100/Filesize);
    lastUpdate = newtime;
    lastBytesSent = BytesSent;
}

void FileTransfertWidget::onFileTransferCancelled(int FriendId, int FileNum, ToxFile::FileDirection Direction)
{
    if (FileNum != fileNum || FriendId != friendId || Direction != direction)
            return;
    buttonLayout->setContentsMargins(0,0,0,0);
    disconnect(topright);
    disconnect(Widget::getInstance()->getCore(),0,this,0);
    progress->hide();
    speed->hide();
    eta->hide();
    topright->hide();
    bottomright->hide();
    QPalette whiteText;
    whiteText.setColor(QPalette::WindowText, Qt::white);
    filename->setPalette(whiteText);
    size->setPalette(whiteText);
    this->setObjectName("error");
    this->style()->polish(this);

    emit needsRedrawing();
}

void FileTransfertWidget::onFileTransferFinished(ToxFile File)
{
    if (File.fileNum != fileNum || File.friendId != friendId || File.direction != direction)
            return;
    topright->disconnect();
    disconnect(Widget::getInstance()->getCore(),0,this,0);
    progress->hide();
    speed->hide();
    eta->hide();
    topright->hide();
    bottomright->hide();
    buttonLayout->setContentsMargins(0,0,0,0);
    QPalette whiteText;
    whiteText.setColor(QPalette::WindowText, Qt::white);
    filename->setPalette(whiteText);
    size->setPalette(whiteText);
    this->setObjectName("success");
    this->style()->polish(this);

    emit needsRedrawing();

    if (File.direction == ToxFile::RECEIVING)
    {
        QFile saveFile(savePath);
        if (!saveFile.open(QIODevice::WriteOnly))
            return;
        saveFile.write(File.fileData);
        saveFile.close();

        QPixmap preview;
        if (preview.loadFromData(File.fileData))
        {
            preview = preview.scaledToHeight(40);
            pic->setPixmap(preview);
        }
    }
}

void FileTransfertWidget::cancelTransfer()
{
    Widget::getInstance()->getCore()->cancelFileSend(friendId, fileNum);
}

void FileTransfertWidget::rejectRecvRequest()
{
    Widget::getInstance()->getCore()->rejectFileRecvRequest(friendId, fileNum);
    onFileTransferCancelled(friendId, fileNum, direction);
}

void FileTransfertWidget::acceptRecvRequest()
{
    QString path = QFileDialog::getSaveFileName(0,tr("Save a file","Title of the file saving dialog"),QDir::currentPath()+'/'+filename->text());
    if (path.isEmpty())
        return;

    savePath = path;

    bottomright->setStyleSheet(pauseFileButtonStylesheet);
    bottomright->disconnect();
    connect(bottomright, SIGNAL(clicked()), this, SLOT(pauseResumeRecv()));
    Widget::getInstance()->getCore()->acceptFileRecvRequest(friendId, fileNum);
}

void FileTransfertWidget::pauseResumeRecv()
{
    Widget::getInstance()->getCore()->pauseResumeFileRecv(friendId, fileNum);
}

void FileTransfertWidget::pauseResumeSend()
{
    Widget::getInstance()->getCore()->pauseResumeFileSend(friendId, fileNum);
}

void FileTransfertWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
