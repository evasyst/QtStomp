#ifndef QTSTOMPWRAPPER_H
#define QTSTOMPWRAPPER_H

#include <QObject>
#include <QtQml>
#include <QtGlobal>
#include <QGuiApplication>
#include <QHash>
#include "qstompconnection.h"
#include "qstompframebodytext.h"

class QQmlEngine;

namespace com { namespace evasyst { namespace QtStomp {

class QStompConnection;


class QtStompWrapper : public QObject
{
    Q_OBJECT
    // http basic authorication header sent to upgrade request
    Q_PROPERTY(QString authorization READ getAuthorization WRITE setAuthorization NOTIFY authorizationChanged)
    Q_PROPERTY(QString endpoint READ getEndpoint WRITE setEndpoint NOTIFY endpointChanged)
    Q_PROPERTY(QString subscribePath READ getSubscribePath WRITE setSubscribePath NOTIFY subscribePathChanged)
    Q_PROPERTY(QString incomingMessage READ getIncomingMessage WRITE setIncomingMessage)
    Q_PROPERTY(QString incomingMsgBody READ getIncomingMsgBody)
    // sets the path to get the subscription id for unsubscribing
    Q_PROPERTY(QString unsubscribePath READ getUnsubscribePath WRITE setUnsubscribePath)
    // maintains a list of STOMP Subscriptions by destination path and id for unsubscribing
    Q_PROPERTY(QHash<QString,QString> subscriptions READ getSubscriptions WRITE setSubscriptions)
    Q_PROPERTY(int stompState READ getStompState WRITE setStompState NOTIFY stompStateChanged)
    Q_PROPERTY(QString subscriptionId READ getSubscriptionId WRITE setSubscriptionId)
    Q_PROPERTY(QVariantMap map READ getMap WRITE setMap NOTIFY mapChanged)
    Q_PROPERTY(QString lastMessageSubscriptionId READ getLastMessageSubscriptionId WRITE setLastMessageSubscriptionId)

public:
    QtStompWrapper(QObject *parent=0);
    ~QtStompWrapper();
    Q_INVOKABLE void openWebSocket();
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE void subscribe();
    Q_INVOKABLE void unsubscribe();
    Q_INVOKABLE void subscribeTo(const QString &destination);
    Q_INVOKABLE void unsubscribeFrom(const QString &destination);
    Q_INVOKABLE void unsubscribeAll();
    Q_INVOKABLE void sendMessage(const QString &msg);
    Q_INVOKABLE QString getFrameMsgHeader(const QString &header);

    enum EStompConnection {
        NotConnected = 0,
        Connecting = 2,
        Connected
    };
    Q_ENUMS(EStompConnection)

    QString getAuthorization() const { return m_authorization; }
    void setAuthorization(const QString &authorization);

    QString getEndpoint() const { return m_endpoint; }
    void setEndpoint(const QString &endpoint);

    QString getSubscriptionId() const { return m_subscription_id; }
    void setSubscriptionId(const QString &subscriptionId);

    QString getLastMessageSubscriptionId() const { return m_last_message_subscription_id; }
    void setLastMessageSubscriptionId(const QString &lastMessageSubscriptionId);

    QString getSubscribePath() const { return m_subscribe_path; }
    void setSubscribePath(const QString &subscribePath);

    QString getUnsubscribePath() const { return m_unsubscribe_path; }
    void setUnsubscribePath(const QString &unsubscribePath);

    QString getIncomingMessage() const { return m_incoming_message; }
    void setIncomingMessage(const QString &incomingMessage);

    QHash<QString,QString> getSubscriptions() const { return m_subscriptions; }
    void setSubscriptions(const QHash<QString,QString> &subscriptions);

    int getStompState() const { return m_stomp_state; }
    void setStompState(int sockState);

    QString getIncomingMsgBody() const;

    static const QString resourceaddr;
    static QtStompWrapper* getInstance();

    QVariantMap getMap() const { return Map; }
    void setMap(QVariantMap map) {
       if(Map != map) {
           Map = map;
           emit mapChanged();
       }
    }
signals:
    void authorizationChanged();
    void endpointChanged();
    void subscribePathChanged();
    void messageRecieved();
    void stompStateChanged();
    void stompErrorReceived();
    void mapChanged();
    // issued when socket is no longer valid
    void offline();
public slots:
    void onStompMessageRecieved(const QString &message);
    void onConnecting();
    void onDisConnecting();

private:
    static QStompConnection* websocketConnection;
    QString m_authorization;
    QString m_subscribe_path;
    QString m_endpoint;
    QString m_incoming_message;
    QString m_unsubscribe_path;
    QString m_subscription_id;
    QHash<QString,QString> m_subscriptions;
    QString m_last_message_subscription_id;
    int m_stomp_state;
    QVariantMap Map;
};
}}}

#endif // QTSTOMPWRAPPER_H
