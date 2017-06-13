#ifndef Q_STOMP_CONNECTION_H
#define Q_STOMP_CONNECTION_H

#include "qstompframe.h"

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <QTimer>

#include <QtNetwork/QAbstractSocket>

class QStompConnectionPrivate;

class QStompConnection : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(QStompConnection);

    public:
        explicit QStompConnection(QObject *parent = 0, int heartBeat = 0);
        ~QStompConnection();

        void setHostName(const QString &hostName);
        void setPort(quint16 port);
        void setLogin(const QString &login);
        void setPassCode(const QString &passCode);
        void setAuthorization(const QString &authorization);

        QString authorization() const;
        QString hostName() const;
        quint16 port() const;
        QString login() const;
        QString passCode() const;
        bool isWSValid() const;

    public Q_SLOTS:
        void connectToServer();
        void disconnectFromServer();
        void sendFrame(const QStompFrame &frame);
        void sendHeartbeat();
        void reConnectToServer();

    protected:
        const QScopedPointer<QStompConnectionPrivate> d_ptr;

    Q_SIGNALS:
        void connected();
        void disconnected();
        void connectFailed();
        void disconnectFailed();

        void connectedFrameReceived(const QStompFrame &frame);
        void errorFrameReceived(const QStompFrame &frame);
        void messageFrameReceived(const QStompFrame &frame);
        void receiptFrameReceived(const QStompFrame &frame);
        void textMessageRecieved(const QString &message);
        void processMessage(const QString &message);

    private Q_SLOTS:
        void onSocketConnected();
        void onSocketDisconnected();
        void onSocketError(QAbstractSocket::SocketError socketError);
        void onConnectedFrameReceived(const QStompFrame &frame);
        void onTextMessageRecieved(const QString &message);
        void onBinaryMessageRecieved(const QByteArray &message);
    private:
        Q_DECLARE_PRIVATE(QStompConnection);
        int m_heartbeat;
        QTimer *hb_timer;
};

#endif // Q_STOMP_CONNECTION_H


