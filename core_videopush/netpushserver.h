#pragma once

#include "videopushhelper.h"
#include "netpushclient.h"

class NetPushServer : public QObject
{
    Q_OBJECT
public:
    explicit NetPushServer(QObject *parent = 0);

private:
    //是否启动
    bool isStart;
    //推流地址
    QString pushUrl;

    //地址集合(地址和对应hash值)
    QHash<QString, QString> urls;
    //推流对象集合
    QList<NetPushClient *> clients;

    //定时器处理启动推流线程
    int pushIndex;
    QTimer *timer;

private slots:
    void startPush();

public:
    //根据播放地址获取推流对象
    NetPushClient *getClient(const QString &mediaUrl);
    //获取指定地址的推流地址
    QString getPushUrl(const QString &mediaUrl);

public slots:
    //添加地址返回唯一标识
    bool addUrl(const QString &url, QString &flag);
    //更新地址
    void updateUrl(const QString &url, const QString &srcFlag, const QString &dstFlag);
    //移除地址
    void removeUrl(const QString &flag);
    //清空地址
    void clearUrl();
    //设置推流地址
    void setPushUrl(const QString &pushUrl);

    //启动推流(指定地址则对单个处理)
    void start(const QString &mediaUrl = QString());
    //停止推流(指定地址则对单个处理)
    void stop(const QString &mediaUrl = QString());

signals:
    //推流开始
    void pushStart(const QString &mediaUrl, int width, int height, int videoStatus, int audioStatus, bool start);
    //推流状态变化
    void pushChanged(const QString &mediaUrl, int state);
};

