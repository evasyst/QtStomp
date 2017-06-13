#include "qstompconnectframe.h"

#include "qstompframe_p.h"

#include <QtCore/QDebug>

class QStompConnectFramePrivate : public QStompFramePrivate {

};

QStompConnectFrame::QStompConnectFrame(const QString &hostName, int heartbeat)
    : QStompFrame(*new QStompConnectFramePrivate, QtStomp::CommandConnect)
{
    setHeader(QtStomp::HeaderAcceptVersion, "1.2");
    setHeader(QtStomp::HeaderHost, hostName);
    setHeader(QtStomp::HeaderHeartbeat, QString("%1,10000").arg(heartbeat));
}

QStompConnectFrame::~QStompConnectFrame()
{

}


