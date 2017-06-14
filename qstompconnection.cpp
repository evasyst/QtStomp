#include "qstompconnection.h"

#include "qstompconnectframe.h"

#include <QtCore/QDebug>

#include <QtNetwork/QTcpSocket>
#include <QWebSocket>
#include <QNetworkRequest>

namespace com { namespace evasyst { namespace QtStomp {

class QStompConnectionPrivate {
    public:
        virtual ~QStompConnectionPrivate() { }
        QString hostName;
        quint16 port;
        QString login;
        QString passCode;
        QString authorization;
        QNetworkRequest networkRequest;
        QWebSocket socket;
};

QStompConnection::QStompConnection(QObject *parent, int heartBeat)
    : QObject(parent),
      d_ptr(new QStompConnectionPrivate),
      m_heartbeat(heartBeat),
      hb_timer(NULL)
{
    Q_D(QStompConnection);

    connect(&d->socket, SIGNAL(connected()),
            this, SLOT(onSocketConnected()));
    connect(&d->socket, SIGNAL(disconnected()),
            this, SLOT(onSocketDisconnected()));
    connect(&d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(connectedFrameReceived(QStompFrame)),
            this, SLOT(onConnectedFrameReceived(QStompFrame)));
    connect(&d->socket, SIGNAL(textMessageReceived(QString)),
             this, SLOT(onTextMessageRecieved(QString)));
}

QStompConnection::~QStompConnection()
{
    if (hb_timer) {
        hb_timer->stop();
        //delete hb_timer;
    }
}

QString QStompConnection::hostName() const
{
    Q_D(const QStompConnection);

    return d->hostName;
}

void QStompConnection::setHostName(const QString &hostName)
{
    Q_D(QStompConnection);

    d->hostName = hostName;
}

QString QStompConnection::authorization() const
{
    Q_D(const QStompConnection);

    return d->authorization;
}

void QStompConnection::setAuthorization(const QString &authorization)
{
    Q_D(QStompConnection);

    d->authorization = authorization;
}

quint16 QStompConnection::port() const
{
    Q_D(const QStompConnection);

    return d->port;
}

void QStompConnection::setPort(quint16 port)
{
    Q_D(QStompConnection);

    d->port = port;
}

QString QStompConnection::login() const
{
    Q_D(const QStompConnection);

    return d->login;
}

void QStompConnection::setLogin(const QString &login)
{
    Q_D(QStompConnection);

    d->login = login;
}

QString QStompConnection::passCode() const
{
    Q_D(const QStompConnection);

    return d->passCode;
}
/*
 * Returns true if the socket is ready for reading and writing;
 * otherwise returns false
 */
bool QStompConnection::isWSValid() const
{
    Q_D(const QStompConnection);
    return d->socket.isValid();
}

void QStompConnection::setPassCode(const QString &passCode)
{
    Q_D(QStompConnection);

    d->passCode = passCode;
}

void QStompConnection::connectToServer()
{
    Q_D(QStompConnection);

//    Q_ASSERT(d->socket.state() == QAbstractSocket::UnconnectedState);
    if (d->socket.state() != QAbstractSocket::UnconnectedState) {
        qWarning() << "Attempted to connect a socket that is already in a connected state.";
        return;
    }
    QUrl url = QUrl(d->hostName);
    d->networkRequest.setUrl(url);
    d->networkRequest.setRawHeader("Authorization", d->authorization.toUtf8());
    d->socket.open(d->networkRequest);
}

void QStompConnection::disconnectFromServer()
{
    Q_D(QStompConnection);
//    Q_ASSERT(d->socket.state() == QAbstractSocket::ConnectedState);
    if (d->socket.state() != QAbstractSocket::ConnectedState){
        qWarning() << "Attempted to disconnect a socket that is already in disconnected state.";
        return;
    }

    QStompFrame disconnectFrame(QtStomp::CommandDisconnect);
    sendFrame(disconnectFrame);

    // Needing to close the socket also in order to be able to login again
    d->socket.close();
}

void QStompConnection::onSocketConnected()
{
    Q_D(QStompConnection);
    qDebug() << "Got Connected Signal";
    // TODO: Carry out the STOMP connection process
    QStompConnectFrame connectFrame(d->hostName, m_heartbeat);
    sendFrame(connectFrame);

    // TODO: Only after that is done, emit the connected signal
    Q_EMIT connected();
}

void QStompConnection::onSocketDisconnected()
{
    if (hb_timer) {
        hb_timer->stop();
        delete hb_timer;
        hb_timer = NULL;
    }
    Q_EMIT disconnected();
}

void QStompConnection::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_D(QStompConnection);
    qDebug() << "Got Error Signal: "<<socketError<<" "<<d->socket.errorString();
    // TODO: See what the error is and forward the appropriate signal.
    switch (socketError) {
    case QAbstractSocket::SocketTimeoutError:
    case QAbstractSocket::NetworkError:
    case QAbstractSocket::ProxyConnectionClosedError:
    case QAbstractSocket::OperationError:
    case QAbstractSocket::ConnectionRefusedError:
    case QAbstractSocket::RemoteHostClosedError:
        qDebug()<<"Aborting socket "<<d<<"Current state:"<<d->socket.state();
        if (hb_timer) {
            hb_timer->stop();
            delete hb_timer;
            hb_timer = NULL;
        }
        d->socket.abort();
        reConnectToServer();
        break;
    default:
        //TODO: Ignore or abort as well?
     break;
    }
}

void QStompConnection::sendFrame(const QStompFrame &frame)
{
    Q_D(QStompConnection);
    qDebug() << "Sending Frame: "+frame.encodeFrame();
    // Assert that the socket is open to send frames over.
    if(!this->isWSValid()){
        onSocketDisconnected();
    }
    if (hb_timer) {
        hb_timer->stop();
    }
    d->socket.sendBinaryMessage(frame.encodeFrame());
    if (hb_timer) {
        hb_timer->start();
    }
    //    d->socket.write(frame.encodeFrame());
}

void QStompConnection::sendHeartbeat()
{
    Q_D(QStompConnection);
    if(!this->isWSValid()){
        onSocketDisconnected();
    }
    //qDebug()<<"Sending heartbeat";
    d->socket.sendBinaryMessage(QByteArray("\n"));
}

void QStompConnection::reConnectToServer()
{
    Q_D(QStompConnection);
    if ( d->socket.state() != QAbstractSocket::UnconnectedState )
        d->socket.abort();
    connectToServer();
}

void QStompConnection::onConnectedFrameReceived(const QStompFrame &/*frame*/)
{
    if (m_heartbeat > 0)
    {
        hb_timer = new QTimer(this);
        connect(hb_timer, SIGNAL(timeout()), this, SLOT(sendHeartbeat()));
        hb_timer->setInterval(m_heartbeat);
        hb_timer->start();
    }

}

void QStompConnection::onTextMessageRecieved(const QString &message){
    if ((message.isEmpty()== false ) && ! (message.at(0) == QChar('\n')) )
        qDebug() << "Message Recieved: "+message;
    QStompFrame f(message);
    if (f.getCommand() == QtStomp::CommandConnected)
        connectedFrameReceived(f);
    processMessage(message);
}

void QStompConnection::onBinaryMessageRecieved(const QByteArray &message){
    qDebug() << "Message Recieved: "+message;
}

}}}
