#pragma once

#include "videopushhelper.h"
#include "filepushclient.h"

class FilePushServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit FilePushServer(QObject *parent = 0);
    ~FilePushServer();

protected:
    void incomingConnection(intptr handle);

private:
    //播放模式 0-直接播放 1-下载播放
    int playMode;
    //最大请求数量
    int maxCount;

    //文件对象集合(文件名称和对应hash值)
    QHash<QString, QString> files;
    //推流连接对象集合
    QList<FilePushClient *> clients;

private slots:
    //连接通信线程结束
    void finish();
    //更新传输文件
    void changeFile(const QString &fileName);
    //计算连接数
    void calcConnection();

public:
    //添加文件返回唯一标识
    bool addFile(const QString &file, QString &flag);
    //更新地址
    void updateFile(const QString &file, const QString &srcFlag, const QString &dstFlag);
    //移除文件
    void removeFile(const QString &flag);
    //清空文件
    void clearFile();

    //根据唯一标识找文件(一般访问的时候带的唯一标识)
    QString findFile(const QString &flag);
    //获取文件对应的网址
    QString getHttpUrl(const QString &file);

    //设置播放模式
    void setPlayMode(int playMode);
    //设置最大请求数量
    void setMaxCount(int maxCount);

    //启动服务(监听地址和端口)
    bool start(const QString &ip = QString(), int port = 0);
    //停止服务
    void stop();

signals:
    //发送数据
    void sendData(const QByteArray &data);
    //接收数据
    void receiveData(const QByteArray &data);

    //收到新的连接请求
    void receiveConnection(const QString &address);
    //统计每个文件对应连接数
    void receiveCount(const QList<QString> &names, const QList<int> &counts);
};

