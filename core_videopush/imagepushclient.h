#pragma once

#include "videopushhelper.h"

class ImagePushClient : public QThread
{
    Q_OBJECT
public:
    explicit ImagePushClient(intptr socketDescriptor, QObject *parent = 0);
    ~ImagePushClient();

protected:
    void run();

private:
    //数据锁
    QMutex mutex;
    //停止线程标志位
    volatile bool stopped;

    //唯一标识
    QString flag;
    //连接通信对象
    QTcpSocket *tcpSocket;

    //准备发送的图片队列
    QList<QImage> images;

private slots:
    //读取数据
    void readData();
    //发送数据
    void writeData(const QByteArray &data);

public:
    //获取IP地址
    QString getAddress();

    //设置唯一标识
    void setFlag(const QString &flag);
    //添加图片
    void append(const QImage &image);

    //停止线程
    void stop();

signals:
    //发送数据
    void sendData(const QByteArray &data);
    //接收数据
    void receiveData(const QByteArray &data);

    //通信结束
    void finish();
    //发送数据
    void readyWrite(const QByteArray &data);
};

