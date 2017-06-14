#include <QtCore/QCoreApplication>
#include <QtQml/qqml.h>

#include "qtstompwrapper.h"

namespace com { namespace evasyst { namespace QtStomp {

	static const char* GEL_URI = "com.evasyst.QtStomp";

	static void registerTypes()
	{
        qmlRegisterType<QtStompWrapper>(GEL_URI, 4, 7, "QtStompWrapper");
	    qmlProtectModule(GEL_URI, 1);
	}

	Q_COREAPP_STARTUP_FUNCTION(registerTypes)
} } }
