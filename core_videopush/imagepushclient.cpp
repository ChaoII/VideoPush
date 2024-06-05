#include "imagepushclient.h"
#include <QByteArray>

ImagePushClient::ImagePushClient(intptr socketDescriptor, QObject *parent) : QThread(parent) {
    stopped = false;
    flag = "test";
    //实例化通信对象
    tcpSocket = new QTcpSocket(this);
    //设置描述符为传过来的则相当于就是服务端接收到的连接
    tcpSocket->setSocketDescriptor(socketDescriptor);
    //读取数据
    connect(tcpSocket, &QTcpSocket::readyRead, this, &ImagePushClient::readData);
    //断开连接
    connect(tcpSocket, &QTcpSocket::disconnected, this, &ImagePushClient::finish);
    //发送数据
    connect(this, &ImagePushClient::readyWrite, this, &ImagePushClient::writeData);
}

ImagePushClient::~ImagePushClient() {
    if (this->isRunning()) {
        this->disconnect();
        this->stop();
    }
}

void ImagePushClient::run() {
    while (!stopped) {
        if (images.count() > 0) {
            mutex.lock();
            QImage image = images.takeFirst();
            mutex.unlock();

            //图片转数据比较耗时/放在线程中执行
            QByteArray imageData;
            QBuffer buffer(&imageData);
            image.save(&buffer, "JPG");

            //构建要发送的数据
            QByteArray data;
            data.append("Content-Type: image/jpeg\r\n");
            data.append(QString("Content-length: %1\r\n\r\n").arg(imageData.size()).toStdString());
            data.append(imageData);
            data.append(QString("\r\n--%1\r\n").arg(flag).toStdString());
            emit readyWrite(data);
        }

        msleep(1);
    }

    stopped = false;
}

void ImagePushClient::readData() {
    QByteArray buffer = tcpSocket->readAll();
    emit receiveData(buffer);
    if (buffer.contains("favicon.ico")) {
        emit finish();
        return;
    }

    //立即应答告诉请求方接下来我要传输二进制视频流(其实就是一张张图片)
    QByteArray data;
    data.append("HTTP/1.1 200 OK\r\n");
    data.append("Content-Type: multipart/x-mixed-replace;");
    data.append(QString("boundary=%1\r\n").arg(flag).toStdString());
    data.append(QString("\r\n--%1\r\n").arg(flag).toStdString());
    tcpSocket->write(data);
    emit sendData(data);
}

void ImagePushClient::writeData(const QByteArray &data) {
    tcpSocket->write(data);
}

QString ImagePushClient::getAddress() {
    return tcpSocket->peerAddress().toString();
}

void ImagePushClient::setFlag(const QString &flag) {
    this->flag = flag;
}

void ImagePushClient::append(const QImage &image) {
    if (images.count() < 100) {
        mutex.lock();
        images << image;
        mutex.unlock();
    }
}

void ImagePushClient::stop() {
    this->images.clear();
    this->stopped = true;
    this->wait();
}
