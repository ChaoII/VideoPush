#include "netpushserver.h"

NetPushServer::NetPushServer(QObject *parent) : QObject(parent) {
    isStart = false;
    pushUrl = "rtmp://127.0.0.1:6908";

    //定时器处理启动推流线程
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &NetPushServer::startPush);
    timer->setInterval(50);
}

void NetPushServer::startPush() {
    if (pushIndex >= clients.count()) {
        timer->stop();
        return;
    }

    clients.at(pushIndex)->start();
    pushIndex++;
}

NetPushClient *NetPushServer::getClient(const QString &mediaUrl) {
    NetPushClient *pushClient = nullptr;
    for (auto &client: clients) {
        if (client->getMediaUrl() == mediaUrl) {
            pushClient = client;
            break;
        }
    }
    return pushClient;
}

QString NetPushServer::getPushUrl(const QString &mediaUrl) {
    //补充完整的推流前缀
    QString url = pushUrl;
    if (!url.endsWith("/")) {
        url = url + "/";
    }

    //根据播放地址找到唯一标识
    QHash<QString, QString>::const_iterator i = urls.constBegin();
    while (i != urls.constEnd()) {
        if (i.value() == mediaUrl) {
            url = url + i.key();
            break;
        }
        ++i;
    }

    //如果是udp推流可能唯一标识符后面指定的端口需要调整
    if (url.startsWith("udp://") && url.contains("/:")) {
        url.replace("/:", ":");
    }

    return url;
}

bool NetPushServer::addUrl(const QString &url, QString &flag) {
    //如果指定了值则采用指定的否则采用生成的
    if (flag.isEmpty()) {
        flag = VideoPushHelper::getCryptoString(url);
    }

    if (!urls.contains(flag)) {
        urls[flag] = url;
        //一个地址对应一个推流类
        auto client = new NetPushClient;
        connect(client, &NetPushClient::pushStart, this, &NetPushServer::pushStart);
        connect(client, &NetPushClient::pushChanged, this, &NetPushServer::pushChanged);
        client->setMediaUrl(url);
        client->setPushUrl(getPushUrl(url));
        //启动服务状态下自动启动线程
        if (isStart) {
            client->start();
        }

        clients << client;
        return true;
    }

    QMessageBox::critical(nullptr, "错误", "推流码重复, 请重新填写!");
    return false;
}

void NetPushServer::updateUrl(const QString &url, const QString &srcFlag, const QString &dstFlag) {
    //旧的不存在不用继续/新的存在说明重复了也不用继续
    if (!urls.contains(srcFlag) || urls.contains(dstFlag)) {
        return;
    }

    //先移除旧的再添加新的
    urls.remove(srcFlag);
    urls[dstFlag] = url;

    //更新推流类对应的推流码
    for (auto &client: clients) {
        if (client->getMediaUrl() == url) {
            client->setPushUrl(getPushUrl(url));
            break;
        }
    }
}

void NetPushServer::removeUrl(const QString &flag) {
    urls.remove(flag);
    for (NetPushClient *client: clients) {
        if (client->getPushUrl().endsWith(flag)) {
            client->stop();
            client->deleteLater();
            clients.removeOne(client);
            break;
        }
    }
}

void NetPushServer::clearUrl() {
    //释放所有资源
    for (auto &client: clients) {
        client->stop();
        client->deleteLater();
    }

    clients.clear();
    urls.clear();
}

void NetPushServer::setPushUrl(const QString &pushUrl) {
    this->pushUrl = pushUrl;
    //需要立即更新所有的推流地址
    for (auto &client: clients) {
        QString mediaUrl = client->getMediaUrl();
        client->setPushUrl(getPushUrl(mediaUrl));
    }
}

void NetPushServer::start(const QString &mediaUrl) {
    isStart = true;
    if (mediaUrl.isEmpty()) {
#if 0
        for (auto &client: clients) {
            client->start();
        }
#else
        //用定时器排队启动推流线程/防止界面卡主
        pushIndex = 0;
        timer->start();
#endif
    } else {
        auto client = getClient(mediaUrl);
        if (client) {
            client->start();
        }
    }
}

void NetPushServer::stop(const QString &mediaUrl) {
    isStart = false;
    if (mediaUrl.isEmpty()) {
        for (auto &client: clients) {
            client->stop();
        }
    } else {
        auto client = getClient(mediaUrl);
        if (client) {
            client->stop();
        }
    }
}
