﻿#pragma once

#include <QtGui>
#include <QtNetwork>
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
typedef int intptr;
#else
typedef qintptr intptr;
#endif

#pragma execution_character_set("utf-8")

#ifndef TIMEMS
#define TIMEMS qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))
#endif
#ifndef TIME
#define TIME qPrintable(QTime::currentTime().toString("HH:mm:ss"))
#endif
#ifndef QDATE
#define QDATE qPrintable(QDate::currentDate().toString("yyyy-MM-dd"))
#endif
#ifndef QTIME
#define QTIME qPrintable(QTime::currentTime().toString("HH-mm-ss"))
#endif
#ifndef DATETIME
#define DATETIME qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
#endif
#ifndef STRDATETIME
#define STRDATETIME qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss"))
#endif
#ifndef STRDATETIMEMS
#define STRDATETIMEMS qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz"))
#endif

