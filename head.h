#pragma once

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QtWidgets>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

#include <QtWidgets>

#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

#include <QtCore5Compat/QtCore5Compat>

#endif

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#ifdef Q_OS_ANDROID
#include <QtAndroidExtras>
#endif
#endif

#pragma execution_character_set("utf-8")
#define TIMEMS qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))
#define DATETIME qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))

#include "app/appconfig.h"
#include "app/appevent.h"
#include "core_helper/appdata.h"
