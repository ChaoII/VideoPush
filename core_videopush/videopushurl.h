#pragma once

#include "videopushhead.h"

class VideoPushUrl
{
public:
    //获取推流地址
    static QStringList getUrls(const QStringList &ips);
    //判断主机地址及端口是否在线
    static bool hostLive(const QString &hostName, int port, int timeout = 1000);

    //流媒体服务器信息(服务类型/拉流类型/拉流端口)
    static QList<QString> listPushType;
    static QList<QString> listPullType;
    static QList<int> listPullPort;

    //初始化流媒体服务器信息
    static void initServerInfo();
    static void initServerInfo(const QString &fileName);

    //获取流媒体服务器类型
    static QStringList getPushType();
    //获取推流地址对应的资源目录
    static QString getPushPath(const QString &pushUrl);
    //获取流媒体服务器推流或拉流端口
    static int getPullPort(const QString &pushType, const QString &pullType);
    //获取推流完整地址
    static QString getPushUrl(const QString &pushType, const QString &pushMode, const QString &pushUrl, const QString &pushHost);

    //是否采用liveplayer播放器
    static bool useLivePlayer(const QString &pushType, const QString &pullType);
    //校验拉流类型
    static void checkPullType(const QString &pushType, QString &pullType);
    //替换主机地址
    static void replaceUrl(const QString &dstUrl, const QString hostName, QString &srcUrl);

    //根据流媒体服务器类型和拉流类型获取拉流地址
    static QString getPullUrl(const QString &pushUrl, const QString &pushType, const QString &pullType, const QString &host, const QString &flag);
    //根据流媒体服务器类型获取不同的四种拉流类型
    static void getPullUrl(const QString &pushUrl, const QString &pushType, const QString &host, const QString &flag, QString &url1, QString &url2, QString &url3, QString &url4);

    //生成测试网页预览
    static int maxCount;
    static void getPercent(int count, int &row, int &percent);
    static void preview(const QString &httpUrl, const QString &fileName, quint8 number, const QStringList &urls, bool liveplayer);

};

