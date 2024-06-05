#include "videopushhelper.h"

#ifdef filepush

#include "filepushserver.h"

#endif
#ifdef netpush

#include "ffmpegutil.h"
#include "netpushserver.h"

#endif

QHash<QString, QString> VideoPushHelper::mimeType = QHash<QString, QString>();

QString VideoPushHelper::getContentType(const QString &suffix) {
    if (mimeType.isEmpty()) {
        //视频类型
        mimeType["asf"] = "video/x-ms-asf";
        mimeType["asx"] = "video/x-ms-asf";
        mimeType["avi"] = "video/avi";
        mimeType["ivf"] = "video/x-ivf";
        mimeType["m1v"] = "video/x-mpeg";
        mimeType["m2v"] = "video/x-mpeg";
        mimeType["m4e"] = "video/mpeg4";
        mimeType["movie"] = "video/x-sgi-movie";
        mimeType["mp2v"] = "video/mpeg";
        mimeType["mp4"] = "video/mpeg4";
        mimeType["mpa"] = "video/x-mpg";
        mimeType["mpe"] = "video/x-mpeg";
        mimeType["mpeg"] = "video/mpg";
        mimeType["mpg"] = "video/mpg";
        mimeType["mps"] = "video/x-mpeg";
        mimeType["mpv"] = "video/mpg";
        mimeType["mpv2"] = "video/mpeg";
        mimeType["rv"] = "video/vnd.rn-realvideo";
        mimeType["wm"] = "video/x-ms-wm";
        mimeType["wmv"] = "video/x-ms-wmv";
        mimeType["wmx"] = "video/x-ms-wmx";
        mimeType["wvx"] = "video/x-ms-wvx";

        //音频类型
        mimeType["acp"] = "audio/x-mei-aac";
        mimeType["aif"] = "audio/aiff";
        mimeType["aiff"] = "audio/aiff";
        mimeType["aifc"] = "audio/aiff";
        mimeType["au"] = "audio/basic";
        mimeType["la1"] = "audio/x-liquid-file";
        mimeType["lavs"] = "audio/x-liquid-secure";
        mimeType["lmsff"] = "audio/x-la-lms";
        mimeType["m3u"] = "audio/mpegurl";
        mimeType["midi"] = "audio/mid";
        mimeType["mid"] = "audio/mid";
        mimeType["mp2"] = "audio/mp2";
        mimeType["mp3"] = "audio/mp3";
        mimeType["mp4"] = "audio/mp4";
        mimeType["mnd"] = "audio/x-musicnet-download";
        mimeType["mp1"] = "audio/mp1";
        mimeType["mns"] = "audio/x-musicnet-stream";
        mimeType["mpga"] = "audio/rn-mpeg";
        mimeType["pls"] = "audio/scpls";
        mimeType["ra"] = "audio/vnd.rn-realaudio";
        mimeType["ram"] = "audio/x-pn-realaudio";
        mimeType["rmi"] = "audio/mid";
        mimeType["rmm"] = "audio/x-pn-realaudio";
        mimeType["rpm"] = "audio/x-pn-realaudio-plugin";
        mimeType["snd"] = "audio/basic";
        mimeType["wav"] = "audio/wav";
        mimeType["wax"] = "audio/x-ms-wax";
        mimeType["wma"] = "audio/x-ms-wma";
        mimeType["xpl"] = "audio/scpls";

        //图片类型
        mimeType["fax"] = "image/fax";
        mimeType["gif"] = "image/gif";
        mimeType["ico"] = "image/x-ico";
        mimeType["jfif"] = "image/jpeg";
        mimeType["jpe"] = "image/jpeg";
        mimeType["jpeg"] = "image/jpeg";
        mimeType["jpg"] = "image/jpeg";
        mimeType["net"] = "image/pnetvue";
        mimeType["png"] = "image/png";
        mimeType["rp"] = "image/vnd.rn-realpix";
        mimeType["tif"] = "image/tiff";
        mimeType["tiff"] = "image/tiff";
        mimeType["wbmp"] = "image/vnd.wap.wbmp";
    }

    return mimeType.value(suffix, "Unknown");
}

QString VideoPushHelper::getSize(quint64 size) {
    //转换成单位 KB MB GB 更直观
    QString flag = "KB";
    double value = (double) size / 1024;

    if (value > 1024) {
        value = value / 1024;
        flag = "MB";
    }

    if (value > 1024) {
        value = value / 1024;
        flag = "GB";
    }

    return QString("%1 %2").arg(QString::number(value, 'f', 2)).arg(flag);
}

quint8 VideoPushHelper::cryptoType = 0;
QString VideoPushHelper::cryptoFlag = "127.0.0.1";

QString VideoPushHelper::getCryptoString(const QString &url) {
    QString data = url;
    if (cryptoType == 1) {
        data = url + cryptoFlag;
    } else if (cryptoType == 2) {
        data = QUuid::createUuid().toString();
    }

    return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Md5).toHex();
}

bool VideoPushHelper::checkCount(bool &isCritical, int maxCount, int rowCount) {
#ifdef betaversion
    if (rowCount >= maxCount) {
        if (!isCritical) {
            isCritical = true;
            QMessageBox::critical(0, "错误", QString("试用版最多同时推流 %1 路, 请联系作者(微信feiyangqingyun)购买源码编译正式版!").arg(maxCount));
        }
        return false;
    }
    isCritical = false;
#endif
    return true;
}

void VideoPushHelper::addFile(QTableWidget *tableWidget, const QString &file, FilePushServer *pushServer,
                              const QString &flag) {
    //获取当前行数
    int row = tableWidget->rowCount();
    //校验数量
    static bool isCritical = false;
    if (!checkCount(isCritical, 6, row)) {
        return;
    }

    //不存在则不用继续
    if (!QFile(file).exists()) {
        return;
    }

    //加入到服务中
    QString flag2 = flag;
#ifdef filepush
    if (!pushServer->addFile(file, flag2)) {
        return;
    }
#endif

    //新插入一行
    tableWidget->insertRow(row);

    //插入文件大小
    qint64 fileSize = QFile(file).size();
    auto itemSize = new QTableWidgetItem;
    itemSize->setText(getSize(fileSize));
    itemSize->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    tableWidget->setItem(row, 1, itemSize);

    //插入访问数
    auto itemCount = new QTableWidgetItem;
    itemCount->setText("0");
    itemCount->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    tableWidget->setItem(row, 2, itemCount);

    //插入标识符和文件名
    tableWidget->setItem(row, 0, new QTableWidgetItem(flag2));
    tableWidget->setItem(row, 3, new QTableWidgetItem(file));

    //第一列自动按照内容调整宽度
    tableWidget->resizeColumnToContents(0);
}

void VideoPushHelper::updateFile(QTableWidget *tableWidget, int row, const QString &srcFlag, const QString &dstFlag,
                                 const QString &file, FilePushServer *pushServer) {
    if (srcFlag.isEmpty() || dstFlag.isEmpty() || srcFlag == dstFlag) {
        return;
    }

    //更新单元格
    tableWidget->item(row, 0)->setText(dstFlag);
    //更新推流对象
#ifdef filepush
    pushServer->updateFile(file, srcFlag, dstFlag);
#endif
}

void VideoPushHelper::readFile(QTableWidget *tableWidget, const QString &fileName, FilePushServer *pushServer) {
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        while (!file.atEnd()) {
            QString content = file.readLine();
            content = content.trimmed();
            content.replace("\r", "");
            content.replace("\n", "");
            if (content.isEmpty()) {
                continue;
            }

            QStringList list = content.split(",");
            if (list.count() >= 2) {
                //过滤下不存在的文件
                const QString &name = list.at(1);
                if (QFile(name).exists()) {
                    addFile(tableWidget, name.trimmed(), pushServer, list.at(0).trimmed());
                }
            }
        }
    }
}

void VideoPushHelper::writeFile(QTableWidget *tableWidget, const QString &fileName) {
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        int count = tableWidget->rowCount();
        for (int i = 0; i < count; ++i) {
            QString flag = tableWidget->item(i, 0)->text();
            QString name = tableWidget->item(i, 3)->text();
            QString content = flag.trimmed() + "," + name.trimmed() + "\n";
            file.write(content.toUtf8());
        }
    }
}

int VideoPushHelper::writeAddress(const QString &address, const QString &fileName) {
    int count = 0;
    bool exist = false;
    QFile file(fileName);
    if (file.open(QFile::ReadWrite | QFile::Text)) {
        QStringList list;
        while (!file.atEnd()) {
            QString line = file.readLine();
            line.replace("\n", "");
            if (line.isEmpty()) {
                continue;
            }

            count++;
            list << line;
            if (!address.isEmpty() && line.endsWith(address)) {
                exist = true;
            }
        }

        //不存在则插入到最前面
        if (!address.isEmpty() && !exist) {
            QString text = QString("%1, %2").arg(DATETIME).arg(address);
            list.insert(0, text);
            text = list.join("\n");
            file.resize(0);
            file.write(text.trimmed().toUtf8());
        }
    }

    return count;
}

void VideoPushHelper::addLoop(QTableWidget *tableWidget, const QString &fileName) {
    //获取当前行数
    int row = tableWidget->rowCount();
    //校验数量
    static bool isCritical = false;
    if (!checkCount(isCritical, 1, row)) {
        return;
    }

    qint64 duration = 0;
    if (QFile(fileName).exists()) {
        //如果不统计时长则注释掉下面这行就行
#ifdef netpush
        duration = FFmpegUtil::getDuration(fileName);
#endif
    }

    QString min = QString("%1").arg(duration / 60, 2, 10, QChar('0'));
    QString sec = QString("%2").arg(duration % 60, 2, 10, QChar('0'));
    QString time = QString("%1 分 %2 秒").arg(min).arg(sec);

    //新插入一行
    tableWidget->insertRow(row);
    //插入文件时长
    QTableWidgetItem *itemTime = new QTableWidgetItem(time);
    itemTime->setData(Qt::UserRole, duration);
    itemTime->setTextAlignment(Qt::AlignCenter);
    tableWidget->setItem(row, 0, itemTime);
    //插入文件名称
    tableWidget->setItem(row, 1, new QTableWidgetItem(fileName));
}

void VideoPushHelper::readLoop(QTableWidget *tableWidget, const QString &fileName) {
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
            if (list.count() == 1) {
                addLoop(tableWidget, list.at(0).trimmed());
            }
        }
    }
}

quint64 VideoPushHelper::writeLoop(QTableWidget *tableWidget, const QString &fileName) {
    quint64 duration = 0;
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        int count = tableWidget->rowCount();
        for (int i = 0; i < count; ++i) {
            duration += tableWidget->item(i, 0)->data(Qt::UserRole).toInt();
            QString name = tableWidget->item(i, 1)->text();
            QString content = name.trimmed() + "\n";
            file.write(content.toUtf8());
        }
    }
    return duration;
}

bool VideoPushHelper::hideUser = false;

void
VideoPushHelper::addUrl(QTableWidget *tableWidget, const QString &url, NetPushServer *pushServer, const QString &flag) {
    if (url.isEmpty()) {
        return;
    }

    //获取当前行数
    int row = tableWidget->rowCount();
    //校验数量
    static bool isCritical = false;
    if (!checkCount(isCritical, 4, row)) {
        return;
    }

    //加入到服务中
    QString flag2 = flag;
#ifdef netpush
    if (!pushServer->addUrl(url, flag2)) {
        return;
    }
#endif

    //新插入一行
    tableWidget->insertRow(row);

    //插入分辨率
    QTableWidgetItem *itemSize = new QTableWidgetItem;
    itemSize->setText("0 x 0");
    itemSize->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    tableWidget->setItem(row, 1, itemSize);

#if 1
    //插入状态
    QTableWidgetItem *itemStatus = new QTableWidgetItem;
    itemStatus->setText("准备中");
    itemStatus->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    tableWidget->setItem(row, 2, itemStatus);
#else
    //插入按钮
    QPushButton *btn = new QPushButton;
    //设置名称标识方便外面收到信号处理
    btn->setObjectName(QString("btn_%1_%2").arg(row).arg(2));
    btn->setFlat(true);
    btn->setText("启动");
    tableWidget->setCellWidget(row, 2, btn);
#endif

    QFont font;
    font.setBold(true);
#ifdef Q_OS_WIN
    font.setPixelSize(20);
#else
    font.setPixelSize(15);
#endif
    QColor color = QColor(colors.first());

    //插入视频转码状态
    QTableWidgetItem *itemVideo = new QTableWidgetItem;
    itemVideo->setText("●");
    itemVideo->setFont(font);
    itemVideo->setForeground(color);
    itemVideo->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    tableWidget->setItem(row, 3, itemVideo);

    //插入音频转码状态
    QTableWidgetItem *itemAudio = new QTableWidgetItem;
    itemAudio->setText("●");
    itemAudio->setFont(font);
    itemAudio->setForeground(color);
    itemAudio->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    tableWidget->setItem(row, 4, itemAudio);

    //插入标识符
    tableWidget->setItem(row, 0, new QTableWidgetItem(flag2));

    //隐藏用户信息/对应密码字符用字符替代
    QString url2 = url;
    if (hideUser) {
        int index = url2.indexOf("@");
        if (url2.startsWith("rtsp://") && index >= 0) {
            QStringList list = url.mid(0, index).split("/");
            QString userInfo = list.last();
            QString userInfo2;
            int count = userInfo.length();
            for (int i = 0; i < count; ++i) {
                userInfo2.append("*");
            }

            url2 = url2.replace(userInfo, userInfo2);
        }
    }

    //插入拉流地址
    auto itemUrl = new QTableWidgetItem;
    itemUrl->setText(url2);
    itemUrl->setData(Qt::UserRole, url);
    tableWidget->setItem(row, 5, itemUrl);

    //第一列自动按照内容调整宽度
    tableWidget->resizeColumnToContents(0);
}

void VideoPushHelper::updateUrl(QTableWidget *tableWidget, int row, const QString &srcFlag, const QString &dstFlag,
                                const QString &url, NetPushServer *pushServer) {
    if (srcFlag.isEmpty() || dstFlag.isEmpty() || srcFlag == dstFlag) {
        return;
    }

    //更新单元格
    tableWidget->item(row, 0)->setText(dstFlag);
    //更新推流对象
#ifdef netpush
    pushServer->updateUrl(url, srcFlag, dstFlag);
#endif
}

void VideoPushHelper::readUrl(QTableWidget *tableWidget, const QString &fileName, NetPushServer *pushServer) {
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        while (!file.atEnd()) {
            QString content = file.readLine();
            content.replace("\r", "");
            content.replace("\n", "");
            if (content.isEmpty()) {
                continue;
            }

            //为了避免地址中带了逗号(linux系统桌面推流地址有逗号)
            int index = content.indexOf(",");
            if (index > 0) {
                QString url = content.mid(index + 1, content.length());
                QString flag = content.mid(0, index);
                addUrl(tableWidget, url.trimmed(), pushServer, flag.trimmed());
            }
        }
    }
}

void VideoPushHelper::writeUrl(QTableWidget *tableWidget, const QString &fileName) {
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        int count = tableWidget->rowCount();
        for (int i = 0; i < count; ++i) {
            QString flag = tableWidget->item(i, 0)->text();
            QString name = tableWidget->item(i, 5)->data(Qt::UserRole).toString();
            QString content = flag.trimmed() + "," + name.trimmed() + "\n";
            file.write(content.toUtf8());
        }
    }
}

bool VideoPushHelper::existFlag(const QString &fileName, const QString &flag) {
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        while (!file.atEnd()) {
            QString content = file.readLine();
            QString text = content.split(",").first();
            if (text == flag) {
                QMessageBox::critical(0, "错误", "推流码重复, 请重新填写!");
                return true;
            }
        }
    }

    return false;
}

QString VideoPushHelper::getFlag(QTableWidget *tableWidget, int flagType, const QString &pushFlag) {
    QString flag = pushFlag;
    if (flagType == 0) {
        if (!flag.isEmpty()) {
            //取出末尾行对应的标识
            int row = tableWidget->rowCount();
            if (row == 0) {
                flag = QString("%1%2").arg(flag).arg(1);
            } else {
                QString lastFlag = tableWidget->item(row - 1, 0)->text();
                lastFlag.replace(flag, "");
                int index = lastFlag.toInt() + 1;
                flag = QString("%1%2").arg(flag).arg(index);
            }
        }
    } else if (flagType == 2) {
        flag = "";
    }

    return flag;
}

QList<QString> VideoPushHelper::colors = QList<QString>() << "#A0A0A4" << "#282D30" << "#22A3A9" << "#D64D54";

void VideoPushHelper::setStatus(QTableWidgetItem *item, int status) {
    int index = (status >= colors.count() ? 0 : status);
    item->setForeground(QColor(colors.at(index)));
}

void VideoPushHelper::setStatus(QTableWidget *tableWidget, int row, int state, bool start) {
    //可能是按钮
    auto btn = (QPushButton *) tableWidget->cellWidget(row, 2);
    if (btn) {
        if (state == 0) {
            btn->setText("停止");
        } else if (state == 1) {
            btn->setText("停止");
        } else if (state == 2) {
            btn->setText("停止");
        } else {
            btn->setText("启动");
        }
    } else {
        QTableWidgetItem *item = tableWidget->item(row, 2);
        if (state == 0) {
            item->setText("推流中");
        } else if (state == 1) {
            item->setText(start ? "重推中" : "准备中");
        } else if (state == 2) {
            item->setText("重推中");
        } else {
            item->setText("准备中");
        }
    }
}

void VideoPushHelper::initTip(QLabel *label, int fontSize) {
    static QString tip;
    if (tip.isEmpty()) {
        QStringList list;
        list << QString("<font color='%1' size='%2'>●</font> 表示没有输入流").arg(colors.at(0)).arg(fontSize);
        list << QString("<font color='%1' size='%2'>●</font> 表示没有输出流").arg(colors.at(1)).arg(fontSize);
        list << QString("<font color='%1' size='%2'>●</font> 表示原数据推流").arg(colors.at(2)).arg(fontSize);
        list << QString("<font color='%1' size='%2'>●</font> 表示转码后推流").arg(colors.at(3)).arg(fontSize);
        tip = list.join("    ");
    }

    label->setText(tip + " / " + DATETIME);
}
