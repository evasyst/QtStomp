#ifndef Q_STOMP_CONNECT_FRAME_H
#define Q_STOMP_CONNECT_FRAME_H

#include "qstompframe.h"

namespace com { namespace evasyst { namespace QtStomp {

class QStompConnectFramePrivate;

class QStompConnectFrame : public QStompFrame {

    public:
//        QStompConnectFrame(const QString &hostName, const QString &login, const QString &passCode);
    QStompConnectFrame(const QString &hostName, int heartbeat = 0);

    virtual ~QStompConnectFrame();

    protected:
        QStompConnectFrame(QStompConnectFramePrivate &dd);

    private:
        Q_DECLARE_PRIVATE(QStompConnectFrame)
};

}}}

#endif // Q_STOMP_CONNECT_FRAME_H


