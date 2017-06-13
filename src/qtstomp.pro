TEMPLATE = app

QT += qml quick websockets
CONFIG += c++11
VERSION = 4.5

SOURCES += main.cpp \
    qstompconnectframe.cpp \
    qstompconnection.cpp \
    qstompframe.cpp \
    qstompframebody.cpp \
    qstompframebodytext.cpp \
    qtstompwrapper.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = qml

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    qstompconnectframe.h \
    qstompconnection.h \
    qstompframe_p.h \
    qstompframe.h \
    qstompframebody.h \
    qstompframebodytext.h \
    qtstomp.h \
    qtstompwrapper.h

DISTFILES += \
    qmldir
