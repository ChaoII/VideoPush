#pragma once

#include "videopushhelper.h"

class FilePushClient : public QThread {
Q_OBJECT

public:
    explicit FilePushClient(intptr socketDescriptor, QObject *parent = nullptr);

    ~FilePushClient() override;

protected:
    void run() override;

private:
    //连接描述符
    intptr socketDescriptor;
    //连接通信对象
    QTcpSocket *tcpSocket;
    //数据队列
    QByteArray buffer;

    //文件对象
    QFile *file;
    //文件名称
    QString fileName;
    //文件大小
    qint64 fileSize;
    //文件类型
    QString fileType;
    //发送字节数量
    qint64 writeByteCount;

    //播放模式 0-直接播放 1-下载播放
    int playMode;

private slots:

    //设置文件
    bool setFile(const QString &fileName);

    //关闭文件
    void closeFile();

    //关闭连接
    void closeSocket();

private slots:

    //读取数据
    void readData();

    //写入数据
    void writeData(qint64 bytes);

    //写文件流
    QByteArray getHeadData(const QString &flag, qint64 startPos, qint64 endPos, qint64 bufferSize);

    void writeData200(qint64 startPos);

    void writeData206(qint64 startPos, qint64 endPos);

public:
    //获取当前通信对应的文件
    QString getFileName();

    //设置播放模式
    void setPlayMode(int playMode);

signals:

    //发送数据
    void sendData(const QByteArray &data);

    //接收数据
    void receiveData(const QByteArray &data);

    //通信结束
    void finish();

    //更新传输文件
    void changeFile(const QString &fileName);

    //收到新的连接请求
    void receiveConnection(const QString &address);
};

