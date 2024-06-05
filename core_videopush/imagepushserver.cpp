#include "imagepushserver.h"

ImagePushServer::ImagePushServer(QObject *parent) : QTcpServer(parent) {

}

void ImagePushServer::incomingConnection(intptr handle) {
    auto client = new ImagePushClient(handle, this);
    emit receiveConnection(client->getAddress());
    QString flag = VideoPushHelper::getCryptoString(QString("name%1").arg(rand() % 100));
    client->setFlag(flag);
    connect(client, &ImagePushClient::finish, this, &ImagePushServer::finish);
    connect(client, &ImagePushClient::sendData, this, &ImagePushServer::sendData);
    connect(client, &ImagePushClient::receiveData, this, &ImagePushServer::receiveData);
    clients << client;
    client->start();
    emit receiveCount(clients.size());
}

void ImagePushServer::finish() {
    //断开的连接从队列中移除
    ImagePushClient *client = (ImagePushClient *) sender();
    clients.removeOne(client);
    client->stop();
    client->deleteLater();
    emit receiveCount(clients.count());
}

QString ImagePushServer::getHttpAddr() {
    return QString("http://%1:%2").arg(this->serverAddress().toString()).arg(this->serverPort());
}

void ImagePushServer::sendImage(const QImage &image) {
    //对所有在线连接发送
            foreach (ImagePushClient *client, clients) {
            client->append(image);
        }
}

bool ImagePushServer::start(const QString &ip, int port) {
    //已经监听则先停止
    if (this->isListening()) {
        this->stop();
    }

    //地址为空则为任意网卡
    QHostAddress addr(ip);
    if (ip.isEmpty()) {
        addr = QHostAddress(QHostAddress::Any);
    }

    //端口为空则自动查找空白端口
    if (port == 0) {
        for (quint16 port = 12345; port < 65535; ++port) {
            if (this->listen(addr, port)) {
                break;
            }
        }
    } else {
        return listen(addr, port);
    }

    return true;
}

void ImagePushServer::stop() {
    //释放所有资源
            foreach (ImagePushClient *client, clients) {
            client->deleteLater();
        }

    clients.clear();
    this->close();
}
