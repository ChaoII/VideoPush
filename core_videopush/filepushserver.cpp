#include "filepushserver.h"

FilePushServer::FilePushServer(QObject *parent) : QTcpServer(parent) {
    playMode = 0;
    maxCount = 1000;
}

FilePushServer::~FilePushServer() {
    this->stop();
}

void FilePushServer::incomingConnection(intptr handle) {
    int count = clients.count();
    if (count <= maxCount) {
        auto client = new FilePushClient(handle, this);
        client->setPlayMode(playMode);
        connect(client, &FilePushClient::finish, this, &FilePushServer::finish);
        connect(client, &FilePushClient::sendData, this, &FilePushServer::sendData);
        connect(client, &FilePushClient::receiveData, this, &FilePushServer::receiveData);
        connect(client, &FilePushClient::changeFile, this, &FilePushServer::changeFile);
        connect(client, &FilePushClient::receiveConnection, this, &FilePushServer::receiveConnection);
        clients << client;
        client->start();
    } else {
        QTcpSocket socket;
        socket.setSocketDescriptor(handle);
        socket.write("HTTP/1.1 200 OK\r\n\r\nMore than the maximum connection!");
        socket.waitForBytesWritten();
        socket.abort();
        qDebug() << TIMEMS << "count" << count << "maxCount" << maxCount;
    }
}

void FilePushServer::finish() {
    auto client = dynamic_cast<FilePushClient *>( sender());
    clients.removeOne(client);
    client->deleteLater();
    this->calcConnection();
}

void FilePushServer::changeFile(const QString &fileName) {
    this->calcConnection();
}

void FilePushServer::calcConnection() {
    QList<int> counts;
    QList<QString> names;
    int count = clients.count();
    for (int i = 0; i < count; ++i) {
        FilePushClient *client = clients.at(i);
        QString name = client->getFileName();
        if (name.isEmpty()) {
            continue;
        }

        int index = names.indexOf(name);
        if (index >= 0) {
            counts[index]++;
        } else {
            names << name;
            counts << 1;
        }
    }
    emit receiveCount(names, counts);
}

bool FilePushServer::addFile(const QString &file, QString &flag) {
    //可以自行改变对应格式(比如后面可以加上拓展名)
    //如果指定了值则采用指定的否则采用生成的
    if (flag.isEmpty()) {
        flag = VideoPushHelper::getCryptoString(file);
    }
    if (!files.contains(flag)) {
        files[flag] = file;
        return true;
    }
    return false;
}

void FilePushServer::updateFile(const QString &file, const QString &srcFlag, const QString &dstFlag) {
    //旧的不存在不用继续/新的存在说明重复了也不用继续
    if (!files.contains(srcFlag) || files.contains(dstFlag)) {
        return;
    }

    //先移除旧的再添加新的
    files.remove(srcFlag);
    files[dstFlag] = file;
}

void FilePushServer::removeFile(const QString &flag) {
    files.remove(flag);
}

void FilePushServer::clearFile() {
    files.clear();
}

QString FilePushServer::findFile(const QString &flag) {
    //去掉末尾的拓展名再去查找
    QString key = flag.split(".").first();
    return files.value(key, "");
}

QString FilePushServer::getHttpUrl(const QString &file) {
    if (!this->isListening()) {
        return "";
    }
    QString http = QString("http://%1:%2").arg(serverAddress().toString()).arg(serverPort());
    //如果名称不为空则查找指定文件的唯一标识作为地址后缀
    if (!file.isEmpty()) {
        //找到文件名对应的唯一标识
        QString flag;
        QHash<QString, QString>::const_iterator i = files.constBegin();
        while (i != files.constEnd()) {
            QString fileName = i.value();
            if (fileName.contains(file)) {
                flag = i.key();
                break;
            }
            ++i;
        }

        if (!flag.isEmpty()) {
            http = QString("%1/%2").arg(http).arg(flag);
            //如果是下载模式则需要带上拓展名区分下
            if (playMode == 1 && file.contains(".")) {
                http = http + "." + file.split(".").last();
            }
        }
    }

    return http;
}

void FilePushServer::setPlayMode(int playMode) {
    this->playMode = playMode;
}

void FilePushServer::setMaxCount(int maxCount) {
    if (maxCount > 0) {
        this->maxCount = maxCount;
    }
}

bool FilePushServer::start(const QString &ip, int port) {
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

void FilePushServer::stop() {
    //释放所有资源
    for (auto &client: clients) {
        client->deleteLater();
    }
    clients.clear();
    this->close();
    this->calcConnection();
}
