#pragma once

#include <QObject>

class UdpServer : public QObject {
Q_OBJECT

public:
    explicit UdpServer(QObject *parent = nullptr);

public slots:

    //启动和停止服务
    void start(int port);

    void stop();
};

