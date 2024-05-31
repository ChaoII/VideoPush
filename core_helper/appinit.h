#pragma once

#include <QObject>

class AppInit : public QObject
{
    Q_OBJECT
public:
    static AppInit *Instance();
    explicit AppInit(QObject *parent = 0);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    static QScopedPointer<AppInit> self;

public slots:
    void start();
};

