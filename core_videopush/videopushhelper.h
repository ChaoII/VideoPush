#pragma once

#include "videopushhead.h"
class FilePushServer;
class NetPushServer;

class VideoPushHelper
{
public:
    //文件类型对照表 http://tools.jb51.net/table/http_content_type http://tool.oschina.net/commons
    static QHash<QString, QString> mimeType;
    //根据拓展名获取文件类型
    static QString getContentType(const QString &suffix);

    //将字节转为 KB MB GB 单位
    static QString getSize(quint64 size);

    //地址转加密字符串
    static quint8 cryptoType;
    static QString cryptoFlag;
    static QString getCryptoString(const QString &url);

    //检验数量
    static bool checkCount(bool &isCritical, int maxCount, int rowCount);

    //插入文件推流信息到表格
    static void addFile(QTableWidget *tableWidget, const QString &file, FilePushServer *pushServer, const QString &flag = QString());
    //更新文件唯一标识到表格
    static void updateFile(QTableWidget *tableWidget, int row, const QString &srcFlag, const QString &dstFlag, const QString &file, FilePushServer *pushServer);

    //读取文件推流信息到表格
    static void readFile(QTableWidget *tableWidget, const QString &fileName, FilePushServer *pushServer);
    //写入文件推流信息到表格
    static void writeFile(QTableWidget *tableWidget, const QString &fileName);

    //写入文件推流地址到文件(返回数量)
    static int writeAddress(const QString &address, const QString &fileName);

    //插入循环推流信息到表格
    static void addLoop(QTableWidget *tableWidget, const QString &fileName);
    //读取循环推流信息到表格
    static void readLoop(QTableWidget *tableWidget, const QString &fileName);
    //写入循环推流信息到文件(返回总时长)
    static quint64 writeLoop(QTableWidget *tableWidget, const QString &fileName);

    //是否隐藏用户信息
    static bool hideUser;
    //插入网络推流地址到表格
    static void addUrl(QTableWidget *tableWidget, const QString &url, NetPushServer *pushServer, const QString &flag = QString());
    //更新网络唯一标识到表格
    static void updateUrl(QTableWidget *tableWidget, int row, const QString &srcFlag, const QString &dstFlag, const QString &url, NetPushServer *pushServer);

    //读取网络推流地址到表格
    static void readUrl(QTableWidget *tableWidget, const QString &fileName, NetPushServer *pushServer);
    //写入网络推流地址到表格
    static void writeUrl(QTableWidget *tableWidget, const QString &fileName);

    //校验唯一标识推流码
    static bool existFlag(const QString &fileName, const QString &flag);
    //获取唯一标识推流码
    static QString getFlag(QTableWidget *tableWidget, int flagType, const QString &pushFlag);

    //设置视音频转码状态
    static QList<QString> colors;
    static void setStatus(QTableWidgetItem *item, int status);
    //设置推流状态
    static void setStatus(QTableWidget *tableWidget, int row, int state, bool start);

    //设置提示信息
    static void initTip(QLabel *label, int fontSize = 5);
};

