#pragma once

#include <QObject>

class AppInit : public QObject {
Q_OBJECT

public:
    static AppInit &Instance() {
        static AppInit appInit;
        return appInit;
    }

private:
    explicit AppInit(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;


public slots:

    void start();
};

