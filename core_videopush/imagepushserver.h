#pragma once

#include "imagepushclient.h"

class ImagePushServer : public QTcpServer {
Q_OBJECT

public:
    explicit ImagePushServer(QObject *parent = nullptr);

protected:
    void incomingConnection(intptr handle) override;

private:
    //连接队列
    QList<ImagePushClient *> clients;

private slots:

    //通信结束
    void finish();

public slots:

    //获取对应的网址
    QString getHttpAddr();

    //发送图片
    void sendImage(const QImage &image);

    //启动服务(监听地址和端口)
    bool start(const QString &ip = QString(), int port = 0);

    //停止服务
    void stop();

signals:

    //发送数据
    void sendData(const QByteArray &data);

    //接收数据
    void receiveData(const QByteArray &data);

    //连接数量变化
    void receiveCount(int count);

    //收到新的连接请求
    void receiveConnection(const QString &address);
};

