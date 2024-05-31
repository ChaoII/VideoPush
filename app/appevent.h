#pragma once

#include "qobject.h"
#include "qscopedpointer.h"

class AppEvent : public QObject
{
    Q_OBJECT

public:
    static AppEvent *Instance();
    explicit AppEvent(QObject *parent = 0);

private:
    static QScopedPointer<AppEvent> self;

signals:
    void exitAll();
};

