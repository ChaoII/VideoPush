#include "filepushclient.h"
#include "filepushserver.h"
#include "core_videopush/http_parser/qhttpparser.h"

FilePushClient::FilePushClient(intptr socketDescriptor, QObject *parent) : QThread(parent)
{
    file = NULL;
    tcpSocket = NULL;
    writeByteCount = 0;
    playMode = 0;
    this->socketDescriptor = socketDescriptor;
}

FilePushClient::~FilePushClient()
{
    //如果处于运行状态则先取消所有信号槽连接并等待退出
    if (this->isRunning()) {
        this->disconnect();
        this->quit();
        this->wait();
    }
}

void FilePushClient::run()
{
    //实例化通信对象
    tcpSocket = new QTcpSocket;
    //设置描述符为传过来的则相当于就是服务端接收到的连接
    tcpSocket->setSocketDescriptor(socketDescriptor);
    emit receiveConnection(tcpSocket->peerAddress().toString());

    //如果开启事件循环这种方式则连接信号槽必须用 Qt::DirectConnection 否则会报提示 Parent is QNativeSocketEngine
    //断开连接
    connect(tcpSocket, SIGNAL(disconnected()), this, SIGNAL(finish()), Qt::DirectConnection);
    //读取数据
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()), Qt::DirectConnection);
    //写入数据
    connect(tcpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(writeData(qint64)), Qt::DirectConnection);
    //启动事件循环
    this->exec();
}

bool FilePushClient::setFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return false;
    }

    //先关闭文件
    this->closeFile();

    //检查是否存在或者大小是否正常
    this->fileName = fileName;
    QFileInfo fileInfo(fileName);
    fileSize = fileInfo.size();
    if (!fileInfo.exists() || fileSize <= 4) {
        return false;
    }

    //获取文件类型
    fileType = VideoPushHelper::getContentType(fileInfo.suffix().toLower());
    file = new QFile(fileName);
    emit changeFile(fileName);
    return file->open(QIODevice::ReadOnly);
}

void FilePushClient::closeFile()
{
    if (file) {
        file->close();
        file->deleteLater();
        file = NULL;
    }
}

void FilePushClient::closeSocket()
{
    if (tcpSocket) {
        tcpSocket->abort();
        tcpSocket->deleteLater();
        tcpSocket = NULL;
    }
}

void FilePushClient::readData()
{
    //GET /后缀 HTTP/1.1
    //Host: 192.168.0.110:6908
    //Connection: keep-alive (一般网页请求是keep-alive/其他都是close)
    //Upgrade-Insecure-Requests: 1
    //User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.198 Safari/537.36
    //Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
    //Accept-Encoding: gzip, deflate
    //Accept-Language: zh-CN,zh;q=0.9,en;q=0.8

    QByteArray data = tcpSocket->readAll();
    buffer.append(data);

    //超过了长度说明数据是垃圾数据
    if (buffer.size() > 1000) {
        buffer.clear();
        return;
    }

    //末尾必须是两个回车换行
    if (!buffer.endsWith("\r\n\r\n")) {
        return;
    }

    //解析请求(解析失败则不用继续)
    QHttpParser parser;
    bool ok = parser.parserRequest(buffer);
    emit receiveData(buffer);
    buffer.clear();
    if (!ok) {
        quit();
        return;
    }

    //不是对应的方法
    if (parser.method() != "GET") {
        quit();
        return;
    }

    //取出后缀地址(如果是请求图标则不用继续)
    QString url = parser.url();
    if (!url.startsWith("/") || url.startsWith("/favicon.ico")) {
        return;
    }

    //根据请求中的唯一标识查找文件
    QString flag = url.mid(1, url.length());
    FilePushServer *server = (FilePushServer *)this->parent();
    QString fileName = server->findFile(flag);
    if (!this->setFile(fileName)) {
        quit();
        return;
    }

    QString range = parser.headerValue("Range");
    if (range.isEmpty()) {
        this->writeData200(0);
        return;
    }

    //Range: bytes=0- / bytes=-1024 / bytes=0-1024
    QStringList list = range.split("=");
    if (list.count() != 2) {
        quit();
        return;
    }

    //取出进度范围
    range = list[1];
    range.replace(" ", "");
    list.clear();
    list = range.split("-");

    //测试下来发现基本上都是 x- 的情况
    qint64 startPos, endPos;
    if (range.startsWith("-")) {
        endPos = fileSize - 1;
        startPos = endPos - list.at(0).toInt();
    } else if (range.endsWith("-")) {
        startPos = list.at(0).toInt();
        endPos = fileSize - 1;
    } else {
        startPos = list.at(0).toInt();
        endPos = list.at(1).toInt();
    }

    this->writeData206(startPos, endPos);
}

void FilePushClient::writeData(qint64 bytes)
{
    writeByteCount -= bytes;
    if (tcpSocket && writeByteCount > 0) {
        qint64 size = 512 * 1024;
        size = tcpSocket->write(file->read(size));
        //qDebug() << TIMEMS << "writeData" << size;
    }
}

QByteArray FilePushClient::getHeadData(const QString &flag, qint64 startPos, qint64 endPos, qint64 bufferSize)
{
    QStringList list;
    list << flag;
    list << "Server: QQ_517216493 WX_feiyangqingyun";
    list << "Cache-control: no-cache";
    list << "Pragma: no-cache";
    list << "Connection: close";
    list << "Accept-Ranges: bytes";
    list << "Access-Control-Allow-Origin: *";
    list << QString("Content-Type: %1").arg(fileType);
    list << QString("Content-Length: %1").arg(bufferSize);
    if (playMode == 1) {
        list << QString("Content-Disposition: attachment;filename=%1").arg(fileName);
    }
    list << QString("Content-Range: bytes %1-%2/%3").arg(startPos).arg(endPos).arg(fileSize);
    //末尾必须加个回车换行
    list << "\r\n";

    QString data = list.join("\r\n");
    return data.toUtf8();
}

void FilePushClient::writeData200(qint64 startPos)
{
    if (!file->isOpen()) {
        return;
    }

    if (startPos >= fileSize) {
        return;
    }

    //文件切换到对应位置
    file->seek(startPos);
    qint64 endPos = fileSize - 1;
    qint64 bufferSize = fileSize - startPos;
    QByteArray data = getHeadData("HTTP/1.1 200 OK", startPos, endPos, bufferSize);

    //计算并发送数据
    writeByteCount = data.size() + (fileSize - startPos);
    tcpSocket->write(data);
    emit sendData(data);
    //qDebug() << TIMEMS << "writeData200";
}

void FilePushClient::writeData206(qint64 startPos, qint64 endPos)
{
    if (!file->isOpen()) {
        return;
    }

    if (startPos >= fileSize || startPos >= endPos) {
        return;
    }

    //文件切换到对应位置
    file->seek(startPos);
    qint64 bufferSize = endPos - startPos + 1;
    QByteArray data = getHeadData("HTTP/1.1 206 Partial Content", startPos, endPos, bufferSize);

    //计算并发送数据
    writeByteCount = data.size() + (fileSize - startPos);
    tcpSocket->write(data);
    emit sendData(data);
    //qDebug() << TIMEMS << "writeData206";
}

QString FilePushClient::getFileName()
{
    return this->fileName;
}

void FilePushClient::setPlayMode(int playMode)
{
    this->playMode = playMode;
}
