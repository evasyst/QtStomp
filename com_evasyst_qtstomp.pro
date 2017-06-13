TEMPLATE = app

QT += qml quick websockets
CONFIG += c++11
VERSION = 4.5

SOURCES += com/evasyst/qtstomp/main.cpp \
    com/evasyst/qtstomp/qstompconnectframe.cpp \
    com/evasyst/qtstomp/qstompconnection.cpp \
    com/evasyst/qtstomp/qstompframe.cpp \
    com/evasyst/qtstomp/qstompframebody.cpp \
    com/evasyst/qtstomp/qstompframebodytext.cpp \
    com/evasyst/qtstomp/qtstompwrapper.cpp

RESOURCES += com_evasyst_qtstomp.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = qml

# Default rules for deployment.
include(com/evasyst/qtstomp/deployment.pri)

HEADERS += \
    com/evasyst/qtstomp/qstompconnectframe.h \
    com/evasyst/qtstomp/qstompconnection.h \
    com/evasyst/qtstomp/qstompframe_p.h \
    com/evasyst/qtstomp/qstompframe.h \
    com/evasyst/qtstomp/qstompframebody.h \
    com/evasyst/qtstomp/qstompframebodytext.h \
    com/evasyst/qtstomp/qtstomp.h \
    com/evasyst/qtstomp/qtstompwrapper.h

DISTFILES += \
    qmldir
