#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "qtstompwrapper.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // slight variation on recommended qpm register function
    // static void registerTypes() {
	//     qmlRegisterType<QtStompWrapper>("io.evasyst.QtStomp", 1, 0, "QtStomp");
	// }
	// Q_COREAPP_STARTUP_FUNCTION(registerTypes) 
    QtStompWrapper * qtStompWrapperReg = QtStompWrapper::getInstance();
    QtStompWrapper::qmlRegisterType();

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
