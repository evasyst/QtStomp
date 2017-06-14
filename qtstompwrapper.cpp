#include "qtstompwrapper.h"
#include "qstompconnection.h"
#include "qstompframebodytext.h"
#include <QtQml/qqml.h>
#include <QtQml/QQmlEngine>
#include <QtCore/QReadWriteLock>

namespace com { namespace evasyst { namespace QtStomp {
// [9.11.16] Update endpoint to point to staging environment, need to develop a routine to change for PROD [jB]
const QString QtStompWrapper::resourceaddr = "ws://localhost:8080/ws";

QStompConnection* QtStompWrapper::websocketConnection = 0;

/**
 * @brief QtStompWrapper::QtStompWrapper
 * @param parent
 */
QtStompWrapper::QtStompWrapper(QObject *parent) : QObject(parent)
{
    websocketConnection = new QStompConnection(NULL,10000);
    m_stomp_state = QtStompWrapper::NotConnected;
    connect(websocketConnection,SIGNAL(connected()),
            this,SLOT(onConnecting()));
    connect(websocketConnection,SIGNAL(disconnected(void)),
            this,SLOT(onDisConnecting()));
    connect(websocketConnection, SIGNAL(processMessage(QString)),
          this, SLOT(onStompMessageRecieved(QString)));
    setEndpoint(QtStompWrapper::resourceaddr);
}

/**
 * @brief QtStompWrapper::~QtStompWrapper
 */
QtStompWrapper::~QtStompWrapper(){
    if (websocketConnection)
        delete websocketConnection;
}

/**
 * @brief QtStompWrapper::setAuthorization
 * @param authorization
 */
void QtStompWrapper::setAuthorization(const QString &authorization){
    m_authorization = authorization;
}

/**
 * @brief QtStompWrapper::setLastMessageSubscriptionId
 * @param lastMessageRecievedSubscriptionId
 */
void QtStompWrapper::setLastMessageSubscriptionId(const QString &lastMessageRecievedSubscriptionId){
    m_last_message_subscription_id = lastMessageRecievedSubscriptionId;
}

/**
 * @brief QtStompWrapper::setEndpoint
 * @param endpoint
 */
void QtStompWrapper::setEndpoint(const QString &endpoint){
    m_endpoint = endpoint;
}

/**
 * @brief QtStompWrapper::setSubscribePath
 * @param subscribePath
 */
void QtStompWrapper::setSubscribePath(const QString &subscribePath){
    m_subscribe_path = subscribePath;
}

/**
 * @brief QtStompWrapper::setSubscriptionId
 * @param subscriptionId
 */
void QtStompWrapper::setSubscriptionId(const QString &subscriptionId){
    m_subscription_id = subscriptionId;
}

/**
 * @brief QtStompWrapper::setUnsubscribePath
 * @param unsubscribePath
 */
void QtStompWrapper::setUnsubscribePath(const QString &unsubscribePath){
    m_unsubscribe_path = unsubscribePath;
}

/**
 * @brief QtStompWrapper::setSubscriptions
 * @param subscriptions
 */
void QtStompWrapper::setSubscriptions(const QHash<QString,QString> &subscriptions){
    m_subscriptions = subscriptions;
}

void QtStompWrapper::setStompState(int sockState){
    m_stomp_state = sockState;
    stompStateChanged();
}

QString QtStompWrapper::getIncomingMsgBody() const
{
    QString body;
    if (!m_incoming_message.isEmpty()) {
        QStompFrame in(m_incoming_message);
        if (in.hasBody())
        {
            body = QString(in.body()->encode());
        }
    }
    return body;
}

/**
 * @brief QtStompWrapper::setIncomingMessage
 * @param incomingMessage
 */
void QtStompWrapper::setIncomingMessage(const QString &incomingMessage){
    m_incoming_message = incomingMessage;
    messageRecieved();
}

/**
 * @brief QtStompWrapper::connect
 */
void QtStompWrapper::openWebSocket(){
    qDebug()<<"QtStompWrapper openWebSocket: "<<websocketConnection;
    if (websocketConnection) {
        websocketConnection->setHostName(m_endpoint);
        websocketConnection->setAuthorization(m_authorization);
        websocketConnection->connectToServer();
    } else {
        qWarning()<<"ERROR: QtStompWrapper tried to used unintialized websocketConnection";
    }

}

/**
 * @brief QtStompWrapper::subscribe
 */
void QtStompWrapper::subscribe(){
    QStompFrame frame(QtStomp::CommandSubscribe);
    QString subId;
    subId.resize(5);
    for(int s = 0; s < 5; ++s){
        subId[s] = QChar('A'+char(qrand() & ('Z'-'A')));
    }
    frame.setHeader(QtStomp::HeaderId, subId);
    frame.setHeader(QtStomp::HeaderDestination, m_subscribe_path);
    websocketConnection->sendFrame(frame);
    m_subscriptions.insert(m_subscribe_path,subId);
    Map.insert(m_subscribe_path,subId);
    setSubscriptionId(subId);
    emit mapChanged();
}

/**
 * @brief QtStompWrapper::unsubscribe
 */
void QtStompWrapper::unsubscribe(){
    qDebug()<<"unsubscribe from dest: "<<m_unsubscribe_path;
    while (m_subscriptions.contains(m_unsubscribe_path))
    {
        QStompFrame frame(QtStomp::CommandUnsubscribe);
        QString id2remove = m_subscriptions.value(m_unsubscribe_path);
        frame.setHeader(QtStomp::HeaderId, id2remove);
        qDebug()<<"Sending unsubscribe: "<<frame.headers();
        websocketConnection->sendFrame(frame);
        m_subscriptions.remove(m_unsubscribe_path);
    }
}

/**
 * @brief QtStompWrapper::subscribeTo
 * @param destination
 */
void QtStompWrapper::subscribeTo(const QString &destination)
{
    qDebug()<<"Subscribing to "<<destination<<" Stomp state:"<<this->getStompState();
    setSubscribePath(destination);
    subscribe();
}

/**
 * @brief QtStompWrapper::unsubscribeFrom
 * @param destination
 */
void QtStompWrapper::unsubscribeFrom(const QString &destination)
{
    this->setUnsubscribePath(destination);
    unsubscribe();

}

/**
 * @brief QtStompWrapper::unsubscribeAll
 */
void QtStompWrapper::unsubscribeAll()
{
    QHashIterator<QString,QString> i(m_subscriptions);
    while(i.hasNext())
    {
        i.next();
        QStompFrame frame(QtStomp::CommandUnsubscribe);
        QString id2remove = i.value();
        frame.setHeader(QtStomp::HeaderId, id2remove);
        qDebug()<<"Sending unsubscribe: "<<frame.headers();
        websocketConnection->sendFrame(frame);
        m_subscriptions.remove(i.key());
    }
}

/**
 * @brief QtStompWrapper::getFrameMsgHeader
 * @param header
 * @return QString
 */
QString QtStompWrapper::getFrameMsgHeader(const QString &header)
{
    if (!m_incoming_message.isEmpty()) {
        QStompFrame frame(this->m_incoming_message);
        return frame.header(header);
    }
    return QString();
}

/**
 * @brief QtStompWrapper::disconnect
 */
void QtStompWrapper::disconnectFromServer(){
   websocketConnection->disconnectFromServer();
}

/**
 * @brief QtStompWrapper::onStompMessageRecieved
 * @param message
 */
void QtStompWrapper::onStompMessageRecieved(const QString &message){
//    qDebug() << "QtStompWrapper MSG received: "+message;
    QStompFrame incomingFrame(message);

//    qDebug() << " Setting subId: " +incomingFrame.header(QtStomp::HeaderSubscription);
    setLastMessageSubscriptionId(incomingFrame.header(QtStomp::HeaderSubscription));
    setIncomingMessage(message);

   if (getStompState() != QtStompWrapper::Connected)
    {
        if (incomingFrame.getCommand() == QtStomp::CommandConnected)
        {
            qDebug()<<"Received connected frame ==> Changing state to "<<QtStompWrapper::Connected;
            setStompState(QtStompWrapper::Connected);
        }
    }
    else if (incomingFrame.getCommand() == QtStomp::CommandError)
    {
        qWarning()<<"ERROR STOMP received: message:"<<incomingFrame.header(QtStomp::HeaderMsg);
        if (incomingFrame.hasBody())
            qDebug()<<incomingFrame.body()->encode();
        emit stompErrorReceived();
    }
}

void QtStompWrapper::onDisConnecting()
{
    qDebug()<<"QtStompWrapper onDisConnecting";
    setStompState(QtStompWrapper::NotConnected);
    // clear saved subscriptions
    QHashIterator<QString,QString> i(m_subscriptions);
    while(i.hasNext())
    {
        i.next();
        m_subscriptions.remove(i.key());
    }
    offline();   // emit offline signal
}

void QtStompWrapper::onConnecting()
{
    qDebug()<<"QtStompWrapper onConnecting";
    setStompState(QtStompWrapper::Connecting);
}

/**
 * @brief QtStompWrapper::sendMessage
 * @param msg
 */
void QtStompWrapper::sendMessage(const QString &msg){
    QStompFrameBody *fb = new QStompFrameBodyText(msg, "application/json");

    QStompFrame frame(QtStomp::CommandSend);
    // need to fix-hardcoding 
    frame.setHeader("destination","/app/someendpoint");
    frame.setBody(fb);

    websocketConnection->sendFrame(frame);
}
}}}
