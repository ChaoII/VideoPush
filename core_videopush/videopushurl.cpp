#include "videopushurl.h"

QStringList VideoPushUrl::getUrls(const QStringList &ips) {
    QStringList urls;
    for (auto &ip: ips) {
        urls << QString("rtmp://%1/stream").arg(ip);
        urls << QString("rtmp://%1:10085/hls").arg(ip);
        urls << QString("rtsp://%1/stream").arg(ip);
        urls << QString("rtsp://%1:8554").arg(ip);
        urls << QString("rtsp://%1:5541").arg(ip);
    }

    urls << "rtmp://live.imtongban.cool/live";
    return urls;
}

bool VideoPushUrl::hostLive(const QString &hostName, int port, int timeout) {
    //局部的事件循环,不卡主界面
    QEventLoop eventLoop;

    //设置超时
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    timer.setSingleShot(true);
    timer.start(timeout);

    //主动测试下连接
    QTcpSocket tcpSocket;
    QObject::connect(&tcpSocket, &QTcpSocket::connected, &eventLoop, &QEventLoop::quit);
    tcpSocket.connectToHost(hostName, port);
    eventLoop.exec();

    //判断最终状态
    bool ok = (tcpSocket.state() == QAbstractSocket::ConnectedState);
    return ok;
}

QList<QString> VideoPushUrl::listPushType = QList<QString>();
QList<QString> VideoPushUrl::listPullType = QList<QString>();
QList<int> VideoPushUrl::listPullPort = QList<int>();

void VideoPushUrl::initServerInfo() {
    listPushType.clear();
    listPullType.clear();
    listPullPort.clear();

    listPushType << "mediamtx" << "mediamtx" << "mediamtx" << "mediamtx";
    listPullType << "rtsp" << "rtmp" << "hls" << "webrtc";
    listPullPort << 8554 << 1935 << 8888 << 8889;

    listPushType << "LiveQing" << "LiveQing" << "LiveQing" << "LiveQing" << "LiveQing";
    listPullType << "rtmp" << "hls" << "flv" << "ws-flv" << "webrtc";
    listPullPort << 10085 << 18000 << 18000 << 18000 << 18000;

    listPushType << "EasyDarwin";
    listPullType << "rtsp";
    listPullPort << 5541;

    listPushType << "EasyDarwin2";
    listPullType << "rtsp" << "rtmp" << "hls" << "flv" << "ws-flv";
    listPullPort << 10054 << 10035 << 10086 << 10086 << 10086;

    listPushType << "nginx-rtmp";
    listPullType << "rtmp";
    listPullPort << 1935;

    listPushType << "ZLMediaKit" << "ZLMediaKit" << "ZLMediaKit" << "ZLMediaKit" << "ZLMediaKit" << "ZLMediaKit";
    listPullType << "rtsp" << "rtmp" << "hls" << "flv" << "ws-flv" << "webrtc";
    listPullPort << 554 << 1935 << 80 << 80 << 80 << 80;

    listPushType << "srs" << "srs" << "srs" << "srs";
    listPullType << "rtmp" << "hls" << "flv" << "webrtc";
    listPullPort << 1935 << 8080 << 8080 << 8080;

    listPushType << "ABLMediaServer" << "ABLMediaServer" << "ABLMediaServer" << "ABLMediaServer" << "ABLMediaServer";
    listPullType << "rtsp" << "rtmp" << "hls" << "flv" << "ws-flv" << "webrtc";
    listPullPort << 554 << 1935 << 9088 << 8088 << 6088 << 8192;
}

void VideoPushUrl::initServerInfo(const QString &fileName) {
    listPushType.clear();
    listPullType.clear();
    listPullPort.clear();

    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        while (!file.atEnd()) {
            QString content = file.readLine();
            content.replace("\r", "");
            content.replace("\n", "");
            if (content.isEmpty()) {
                continue;
            }

            QStringList list = content.split(",");
            if (list.count() == 3) {
                listPushType << list.at(0);
                listPullType << list.at(1);
                listPullPort << list.at(2).toInt();
            }
        }
    }
}

QStringList VideoPushUrl::getPushType() {
    QStringList types;
    for (auto &type: listPushType) {
        if (!types.contains(type)) {
            types << type;
        }
    }
    return types;
}

QString VideoPushUrl::getPushPath(const QString &pushUrl) {
    //("rtsp:", "", "127.0.0.1:5541") ("rtsp:", "", "127.0.0.1:5541", "live") ("rtsp:", "", "127.0.0.1:5541", "live/test")
    QString path = "/";
    QStringList list = pushUrl.split("/");
    int count = list.size();
    //从第三位开始后面所有的都是资源目录
    for (int i = 3; i < count; ++i) {
        path = path + list.at(i) + "/";
    }

    //末尾的斜杠去掉
    return path.mid(0, path.length() - 1);
}

int VideoPushUrl::getPullPort(const QString &pushType, const QString &pullType) {
    int port = 80;
    int count = listPushType.size();
    for (int i = 0; i < count; ++i) {
        if (listPushType.at(i) == pushType && listPullType.at(i) == pullType) {
            port = listPullPort.at(i);
            break;
        }
    }
    return port;
}

QString VideoPushUrl::getPushUrl(const QString &pushType, const QString &pushMode, const QString &pushUrl,
                                 const QString &pushHost) {
    QString type = pushType;
    QString mode = pushMode;
    QString url = pushUrl;
    QString host = pushHost;

    //有些服务只支持一种模式
    if (type == "LiveQing") {
        mode = "rtmp";
    } else if (type == "EasyDarwin") {
        mode = "rtsp";
    }

    int port = VideoPushUrl::getPullPort(type, mode);
    if (type == "LiveQing") {
        url = QString("%1://%2:%3/hls").arg(mode).arg(host).arg(port);
    } else if (type == "EasyDarwin") {
        url = QString("%1://%2:%3").arg(mode).arg(host).arg(port);
    } else {
        url = QString("%1://%2:%3/stream").arg(mode).arg(host).arg(port);
    }

    return url;
}

bool VideoPushUrl::useLivePlayer(const QString &pushType, const QString &pullType) {
    bool liveplayer = true;
    if (pushType == "mediamtx") {
        liveplayer = false;
    } else if (pushType == "ABLMediaServer") {
        if (pullType == "webrtc") {
            liveplayer = false;
        }
    }

    return liveplayer;
}

void VideoPushUrl::checkPullType(const QString &pushType, QString &pullType) {
    //有些流媒体服务器只支持部分拉流类型(速度 webrtc > ws-flv > flv > hls)
    if (pushType == "mediamtx") {
        if (pullType != "hls") {
            pullType = "webrtc";
        }
    } else if (pushType == "EasyDarwin2") {
        if (pullType == "webrtc") {
            pullType = "ws-flv";
        }
    } else if (pushType == "LiveQing") {
        if (pullType == "ws-flv") {
            //pullType = "rtmp";
        }
    } else if (pushType == "ZLMediaKit") {
        if (pullType == "webrtc") {
            pullType = "hls";
        }
    } else if (pushType == "srs") {
        if (pullType != "hls") {
            pullType = "flv";
        }
    } else if (pushType == "ABLMediaServer") {
        if (pullType == "webrtc") {
            //pullType = "hls";
        }
    }
}

void VideoPushUrl::replaceUrl(const QString &dstUrl, const QString hostName, QString &srcUrl) {
    if (!dstUrl.isEmpty()) {
        srcUrl.replace(hostName, dstUrl);
    }
}

//各种拉流协议分析 https://www.cnblogs.com/xi-jie/p/14031604.html
//各种服务程序对比 https://qtchina.blog.csdn.net/article/details/132355847
QString
VideoPushUrl::getPullUrl(const QString &pushUrl, const QString &pushType, const QString &pullType, const QString &host,
                         const QString &flag) {
    //找到对应服务器类型和拉流类型的端口
    int port = getPullPort(pushType, pullType);
    //资源目录(可以为空)
    QString path = getPushPath(pushUrl);
    //去掉特殊字符比如?
    QString name = flag.split("?").first();

    //根据服务器类型获取对应的地址
    QString url = QString("://%1:%2%3/%4").arg(host).arg(port).arg(path).arg(name);
    if (pushType == "mediamtx") {
        //同时支持rtsp/rtmp推拉流(非常棒)
        if (pullType == "rtsp") {
            url = "rtsp" + url;
        } else if (pullType == "rtmp") {
            url = "rtmp" + url;
        } else if (pullType == "hls") {
            url = "http" + url;
        } else if (pullType == "webrtc") {
            url = "http" + url;
        }
    } else if (pushType == "LiveQing") {
        //只支持rtmp推流
        if (pullType == "rtmp") {
            url = QString("rtmp://%1:%2/hls/%3").arg(host).arg(port).arg(name);
        } else if (pullType == "hls") {
            url = QString("http://%1:%2/hls/%3/%3_live.m3u8").arg(host).arg(port).arg(name);
        } else if (pullType == "flv") {
            url = QString("http://%1:%2/flv/hls/%3.flv").arg(host).arg(port).arg(name);
        } else if (pullType == "ws-flv") {
            url = QString("ws://%1:%2/ws-flv/hls/%3.flv").arg(host).arg(port).arg(name);
        } else if (pullType == "webrtc") {
            url = QString("webrtc://%1:%2/rtc/hls/%3").arg(host).arg(port).arg(name);
        }
    } else if (pushType == "EasyDarwin") {
        //只支持rtsp推流拉流
        if (pullType == "rtsp") {
            url = "rtsp" + url;
        }
    } else if (pushType == "EasyDarwin2") {
        //同时支持rtsp/rtmp推拉流(官网2023年底重新发布的)
        if (pullType == "rtsp") {
            url = QString("rtsp://%1:%2%3/%4").arg(host).arg(port).arg(path).arg(name);
        } else if (pullType == "rtmp") {
            url = QString("rtmp://%1:%2%3/%4").arg(host).arg(port).arg(path).arg(name);
        } else if (pullType == "hls") {
            url = QString("http://%1:%2/hls/%3/playlist.m3u8").arg(host).arg(port).arg(name);
        } else if (pullType == "flv") {
            url = QString("http://%1:%2/flv%3/%4.flv").arg(host).arg(port).arg(path).arg(name);
        } else if (pullType == "ws-flv") {
            url = QString("http://%1:%2/ws-flv%3/%4.flv").arg(host).arg(port).arg(path).arg(name);
        }
    } else if (pushType == "nginx-rtmp") {
        //只支持rtmp推流拉流
        if (pullType == "rtmp") {
            url = "rtmp" + url;
        }
    } else if (pushType == "ZLMediaKit") {
        //同时支持rtsp/rtmp推拉流(名气最大/用户最多)
        if (pullType == "rtsp") {
            url = "rtsp" + url;
        } else if (pullType == "rtmp") {
            url = "rtmp" + url;
        } else if (pullType == "hls") {
            url = "http" + url + "/hls.m3u8";
        } else if (pullType == "flv") {
            url = "http" + url + ".live.flv";
        } else if (pullType == "ws-flv") {
            url = "ws" + url + ".live.flv";
        } else if (pullType == "webrtc") {

        }
    } else if (pushType == "srs") {
        //不支持rtsp推流拉流(以前支持/后面都移除了)
        if (pullType == "rtmp") {
            url = "rtmp" + url;
        } else if (pullType == "hls") {
            url = "http" + url + ".m3u8";
        } else if (pullType == "flv") {
            url = "http" + url + ".flv";
        } else if (pullType == "webrtc") {
            url = "webrtc" + url;
        }
    } else if (pushType == "ABLMediaServer") {
        //支持rtsp/rtmp推流拉流(目前还不稳定/兼容性不够好)
        //http://192.168.0.110:8192/webrtcstreamer.html?video=/live/test2
        if (pullType == "rtsp") {
            url = "rtsp" + url;
        } else if (pullType == "rtmp") {
            url = "rtmp" + url;
        } else if (pullType == "hls") {
            url = "http" + url + ".m3u8";
        } else if (pullType == "flv") {
            url = "http" + url + ".flv";
        } else if (pullType == "ws-flv") {
            url = "ws" + url + ".flv";
        } else if (pullType == "webrtc") {
            url = QString("http://%1:%2/webrtcstreamer.html?video=%3/%4").arg(host).arg(port).arg(path).arg(name);
        }
    } else if (pushType == "Monibuca") {
        //支持rtsp/rtmp推流拉流(拉流格式众多/各种插件/性能很强劲/具体有待验证)
        if (pullType == "rtsp") {
            url = "rtsp" + url;
        } else if (pullType == "rtmp") {
            url = "rtmp" + url;
        } else if (pullType == "hls") {
            url = QString("http://%1:%2/hls%3/%4.m3u8").arg(host).arg(port).arg(path).arg(name);
        } else if (pullType == "flv") {
            url = QString("http://%1:%2/hdl%3/%4.flv").arg(host).arg(port).arg(path).arg(name);
        } else if (pullType == "ws-flv") {
            url = QString("ws://%1:%2/jessica%3/%4.flv").arg(host).arg(port).arg(path).arg(name);
        } else if (pullType == "webrtc") {
            url = QString("webrtc://%1:%2/webrtc/play%3/%4").arg(host).arg(port).arg(path).arg(name);
        }
    }

    //如果是udp推流则拉流地址就是推流地址
    if (pushUrl.startsWith("udp://")) {
        url = pushUrl + "/" + flag;
        //可能唯一标识符填了端口号
        if (url.contains("/:")) {
            url.replace("/:", ":");
        }
    }

    return url;
}

void VideoPushUrl::getPullUrl(const QString &pushUrl, const QString &pushType, const QString &host, const QString &flag,
                              QString &url1, QString &url2, QString &url3, QString &url4) {
    QString pullType1 = "rtsp";
    QString pullType2 = "rtmp";
    QString pullType3 = "hls";
    QString pullType4 = "flv";
    if (pushType == "mediamtx") {
        pullType4 = "webrtc";
    } else if (pushType == "LiveQing") {
        pullType1 = "rtmp";
        pullType2 = "hls";
        pullType3 = "flv";
        pullType4 = "ws-flv";
    } else if (pushType == "EasyDarwin") {
        pullType1 = pullType2 = pullType3 = pullType4 = "rtsp";
    } else if (pushType == "nginx-rtmp") {
        pullType1 = pullType2 = pullType3 = pullType4 = "rtmp";
    } else if (pushType == "srs") {
        pullType1 = "rtmp";
        pullType2 = "hls";
        pullType3 = "flv";
        pullType4 = "webrtc";
    }

    url1 = VideoPushUrl::getPullUrl(pushUrl, pushType, pullType1, host, flag);
    url2 = VideoPushUrl::getPullUrl(pushUrl, pushType, pullType2, host, flag);
    url3 = VideoPushUrl::getPullUrl(pushUrl, pushType, pullType3, host, flag);
    url4 = VideoPushUrl::getPullUrl(pushUrl, pushType, pullType4, host, flag);
}

int VideoPushUrl::maxCount = 25;

void VideoPushUrl::getPercent(int count, int &row, int &percent) {
    if (count <= 1) {
        row = 1;
        percent = 100;
    } else if (count <= 4) {
        row = 2;
        percent = 49;
    } else if (count <= 9) {
        row = 3;
        percent = 33;
    } else if (count <= 16) {
        row = 4;
        percent = 24;
    } else if (count <= 25) {
        row = 5;
        percent = 19;
    } else if (count <= 36) {
        row = 6;
        percent = 16;
    } else if (count <= 49) {
        row = 7;
        percent = 14;
    } else if (count <= 64) {
        row = 8;
        percent = 12;
    }
}

//浏览器打开webrtc调试 chrome://webrtc-internals
void VideoPushUrl::preview(const QString &httpUrl, const QString &fileName, quint8 number, const QStringList &urls,
                           bool liveplayer) {
    int count = urls.count();
    int countAll = count * number;
    if (fileName.isEmpty() || number < 1 || count == 0) {
        QMessageBox::critical(0, "错误", "数量或者参数不符合要求!");
        return;
    }

    if (countAll > maxCount) {
        QMessageBox::critical(0, "错误", QString("最大预览通道数量不能超过 %1 路!").arg(maxCount));
        return;
    }

    QStringList list;
    list << "<html>";
    list << "<head>";
    list << "  <meta charset=utf-8>";

    if (liveplayer) {
        list << "  <script src=liveplayer-element.min.js></script>";
        list << "  <style>";
        list << "    html,body,div{margin:0px;padding:0px;}";
        list << "    td{padding:1px 0px 0px 1px;}";
        list << "    table{border-collapse:collapse;border:0px solid #ff0000;text-align:center;}";
        list << "  </style>";
    } else {
        list << "  <style>";
        list << "    html,body,div{margin:1px;padding:0px;}";
        list << "    td{padding:0px 1px 1px 0px;}";
        list << "    table{border-collapse:collapse;border:0px solid #ff0000;text-align:center;}";
        list << "  </style>";
    }

    list << "  <title>推流视频预览</title>";
    list << "</head>";
    list << "<body>";

    //按照宫格排列(数量少自动拉伸填充)
    int row = 1;
    int percent = 0;
    getPercent(countAll, row, percent);

    bool table = true;
    if (table) {
        percent = 100;
    }

    QStringList iframes;
    for (int i = 0; i < count; ++i) {
        QString url = urls.at(i);
        for (int j = 0; j < number; ++j) {
            QString width = (percent == 0 ? "" : QString(" width=%1%").arg(percent));
            QString height = (percent == 0 ? "" : QString(" height=%1%").arg(percent));
            if (liveplayer) {
                iframes << QString("<live-player video-url=%1 live=true></live-player>").arg(url);
            } else {
                //mediamtx的webrtc网页控制还可以传参数控制默认静音等
                //url = url + "?controls=false&muted=false&autoplay=true&playsinline=true";
                iframes << QString("<iframe frameborder=0 src=%1%2%3></iframe>").arg(url).arg(width).arg(height);
            }
        }
    }

    //表格可以设置边距但是在手机上不会自适应调整行列
    if (table) {
        list << "<table width=100% height=100%>";
        int index = 0;
        for (int i = 0; i < countAll; ++i) {
            if (index >= countAll) {
                break;
            }
            list << "  <tr>";
            for (int j = 0; j < row; ++j) {
                if (index >= countAll) {
                    break;
                }
                list << QString("    <td>%1</td>").arg(iframes.at(index));
                index++;
            }
            list << "  </tr>";
        }
        list << "</table>";
    } else {
        for (auto &iframe: iframes) {
            list << ("  " + iframe);
        }
    }

    list << "</body>";
    list << "</html>";

    //如果是当前路径则转成完成路径
    QString name = fileName;
    if (name.startsWith("./")) {
        name.replace("./", qApp->applicationDirPath() + "/");
    }
    QFile file(name);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QString content = list.join("\n");
        file.write(content.toUtf8());
        file.close();
        //如果是在www目录中说明是网站需要直接打开网络地址
        int index = name.indexOf("www", 0, Qt::CaseInsensitive);
        QString url = QString("file:///%1").arg(name);
        if (index >= 0) {
            url = QString("%1%2").arg(httpUrl).arg(name.mid(index + 3, name.length()));
        }
        QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
    }
}
