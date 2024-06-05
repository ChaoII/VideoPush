#pragma once

#include <QObject>

class AppEvent : public QObject {
Q_OBJECT

public:
    static AppEvent &Instance() {
        static AppEvent appEvent;
        return appEvent;
    }

private:
    explicit AppEvent(QObject *parent = nullptr);

signals:

    void exitAll();
};

