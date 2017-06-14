#include "qstompframe.h"
#include "qstompframe_p.h"
#include "qtstomp.h"
#include "qstompframebodytext.h"
#include <QtCore/QDebug>

namespace com { namespace evasyst { namespace QtStomp {

QStompFrame::QStompFrame(QtStomp::Command command)
    : d_ptr(new QStompFramePrivate)
{
    Q_D(QStompFrame);

    d->command = command;
    d->hasBody = false;
    d->receiptRequested = false;
    d->body = 0;
}

QStompFrame::QStompFrame(const QString & frame) : d_ptr(new QStompFramePrivate)
{
    Q_D(QStompFrame);
    d->hasBody = false;
    bool cmd_found = false, body_sect = false;

    QString frameBody;
    QStringList lines = frame.split("\n");
    QStringList headers;
    headers<< QtStomp::HeaderAcceptVersion << QtStomp::HeaderHost << QtStomp::HeaderLogin << QtStomp::HeaderPassCode
           << QtStomp::HeaderHeartbeat << QtStomp::HeaderVersion << QtStomp::HeaderUsername << QtStomp::HeaderDestination
           << QtStomp::HeaderLen << QtStomp::HeaderContent << QtStomp::HeaderId << QtStomp::HeaderSubscription<< QtStomp::HeaderMsg;

    for (int i=0; i< lines.length();++i)
    {
        if (!cmd_found)
        {
            if (QtStomp::CommandString.indexOf(lines.at(i)) != -1)
            {
                d->command = QtStomp::Command(QtStomp::CommandString.indexOf(lines.at(i)));
                cmd_found = true;
                continue;
            }
        }
        else {
            bool isHeader = false;
            if (!body_sect) {
                foreach (const QString &h, headers)
                {
                    //qDebug()<<h<<" "<<lines.at(i);
                    if ( lines.at(i).indexOf(h) != -1 )
                    {
                        QStringList hdr_value = lines.at(i).split(":");
                        if (hdr_value.length() == 2) {
                            isHeader = true;
                            setHeader(hdr_value[0],hdr_value[1]);
                            break;
                        }
                    }
                }
            }

            // !header  && !command
            if (lines.at(i).isEmpty()) {
                body_sect = true;
                continue;
            }

            if (!isHeader && d->headers.contains(QtStomp::HeaderLen))
            {
                int cont_len = header(QtStomp::HeaderLen).toInt();
                //qDebug()<<"Cont len"<<cont_len<<"Line size:"<<lines.at(i).size()<<" totalConsumed"<<frameBody.size();
                if ( (cont_len <= lines.at(i).size()) && (frameBody.size() < cont_len))
                {
                    frameBody += lines.at(i).left(cont_len);
                } else {
                    //body content is divided in number of rows
                    if ( (frameBody.size() + lines.at(i).size()) > cont_len)
                        frameBody += lines.at(i).left(cont_len-frameBody.size());
                    else
                        frameBody += lines.at(i);
                }

            }

        }
    }
    if (frameBody.isEmpty() == false) {
        QString contentType = header(QtStomp::HeaderContent);
        QStompFrameBody *fb;
        if (contentType.isEmpty()) {
            fb = new QStompFrameBodyText(frameBody);
        } else {
            fb = new QStompFrameBodyText(frameBody,contentType);
        }
        setBody(fb);
    }
}

QStompFrame::QStompFrame(QStompFramePrivate &dd, QtStomp::Command command)
    : d_ptr(&dd)
{ 
    Q_D(QStompFrame);

    d->command = command;
    d->hasBody = false;
    d->receiptRequested = false;
    if (d->hasBody) {
        delete d->body;
    }
}

QStompFrame::~QStompFrame()
{

}

void QStompFrame::setHeader(const QString &headerName, const QString &headerValue)
{
    Q_D(QStompFrame);

    d->headers.insert(headerName, headerValue);
}

QString QStompFrame::header(const QString &headerName) const
{
    Q_D(const QStompFrame);

    return d->headers.value(headerName);
}

QMap<QString, QString> QStompFrame::headers() const
{
    Q_D(const QStompFrame);

    return d->headers;
}

bool QStompFrame::hasHeader(const QString &headerName) const
{
    Q_D(const QStompFrame);

    return d->headers.contains(headerName);
}

bool QStompFrame::hasBody() const
{
    Q_D(const QStompFrame);

    return d->hasBody;
}

void QStompFrame::setHasBody(bool hasBody)
{
    Q_D(QStompFrame);

    if (hasBody == true) {
        Q_ASSERT(d->command == QtStomp::CommandSend
              || d->command == QtStomp::CommandMessage
              || d->command == QtStomp::CommandError);
        switch(d->command)
        {
            case QtStomp::CommandSend:
            case QtStomp::CommandMessage:
            case QtStomp::CommandError:
            break;
        default: {
                qWarning() << "This command type does not support having a body.";
                return;
            }
        }
    }

    d->hasBody = hasBody;
}

bool QStompFrame::isReceiptRequested() const
{
    Q_D(const QStompFrame);

    return d->receiptRequested;
}

void QStompFrame::setReceiptRequested(bool receiptRequested)
{
    Q_D(QStompFrame);

    // TODO: Check if this command type supports receipt requested.

    d->receiptRequested = receiptRequested;
}

QtStomp::Command QStompFrame::getCommand()
{
    Q_D(const QStompFrame);

    return d->command;
}

QString QStompFrame::commandString() const
{
    Q_D(const QStompFrame);

    return QtStomp::CommandString[d->command];
}

QStompFrameBody *QStompFrame::body() const
{
    Q_D(const QStompFrame);

    return d->body;
}

void QStompFrame::setBody(QStompFrameBody *body)
{
    Q_D(QStompFrame);

    if (body != 0) {
        setHasBody(true);
        if (hasBody()) {
            d->body = body;
            d->encodedBody = body->encode();
            d->headers.insert("content-length", QString::number(d->encodedBody.length()));
            d->headers.insert("content-type", body->contentType());
        }
    } else {
        d->body = 0;
        d->encodedBody = QByteArray();
        d->headers.remove("content-length");
        d->headers.remove("content-type");
        setHasBody(false);
    }
}

QByteArray QStompFrame::encodeFrame() const
{
    Q_D(const QStompFrame);

    QString head = commandString();
    head.append(QChar(QChar::LineFeed));

    QMapIterator<QString, QString> i(d->headers);
    while(i.hasNext()) {
        i.next();
        head.append(encodeHeader(i.key(), i.value()));
        head.append(QChar(QChar::LineFeed));
    }
    head.append(QChar(QChar::LineFeed));

    QByteArray frame = head.toUtf8();
    frame.append(d->encodedBody);
    frame.append((char)0);
    
    return frame;
}

QString QStompFrame::encodeHeader(QString headerName, QString headerValue) const
{
    headerName.replace(QChar(QChar::CarriageReturn), "\\r");
    headerName.replace(QChar(QChar::LineFeed), "\\n");
    headerName.replace(':', "\\c");
    headerName.replace('\\', "\\\\");

    headerValue.replace(QChar(QChar::CarriageReturn), "\\r");
    headerValue.replace(QChar(QChar::LineFeed), "\\n");
    headerValue.replace(':', "\\c");
    headerValue.replace('\\', "\\\\");

    return headerName + ":" + headerValue;
}

}}}
