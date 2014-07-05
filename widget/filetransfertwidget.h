#ifndef FILETRANSFERTWIDGET_H
#define FILETRANSFERTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include <QTextObjectInterface>

#include "core.h"

struct ToxFile;


/**
 * @brief The FileTransfertWidget class
 *
 * This class should be only used inside a QTextDocument
 * It implements the interface of a QTextObject, but behaves like a QWidget
 * This works around Qt not allowing widgets to be inserted in a QTextDocument
 *
 * The construction is done in intrisicSize(), and the ToxFile to use should be set
 * as a property before intricicSize() is called for the first time.
 *
 * The owner of the QTextDocument is responsible for sending mouse events to this widget.
 * The class does not believe it is being displayed as another widget's children and
 * cannot receive mouse or keyboard events by itself.
 *
 * This also means that called show() on this widget will open a new window to show it.
 * Even if hide() was previously called.
 */
class FileTransfertWidget : public QWidget, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    FileTransfertWidget();
    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);

signals:
    void needsRedrawing();

public slots:
    void onFileTransferInfo(int FriendId, int FileNum, int Filesize, int BytesSent, ToxFile::FileDirection Direction);
    void onFileTransferCancelled(int FriendId, int FileNum, ToxFile::FileDirection Direction);
    void onFileTransferFinished(ToxFile File);

private slots:
    void cancelTransfer();
    void rejectRecvRequest();
    void acceptRecvRequest();
    void pauseResumeRecv();
    void pauseResumeSend();

protected:
    // We have to simulate mouse events, since we're supposed to be text inside the QTextEdit
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private:
    QString getHumanReadableSize(int size);

private:
    QLabel *pic, *filename, *size, *speed, *eta;
    QPushButton *topright, *bottomright;
    QProgressBar *progress;
    QHBoxLayout *mainLayout, *textLayout;
    QVBoxLayout *infoLayout, *buttonLayout;
    QWidget* buttonWidget;
    QDateTime lastUpdate;
    long long lastBytesSent;
    int fileNum;
    int friendId;
    QString savePath;
    ToxFile::FileDirection direction;
    QString stopFileButtonStylesheet, pauseFileButtonStylesheet, acceptFileButtonStylesheet;
    void paintEvent(QPaintEvent *);
    bool initialized;
};

#endif // FILETRANSFERTWIDGET_H
