#include <QScreen>
#include <QCursor>
#include "deviceinfohelper.h"

#ifdef multimedia

#include <QtMultimedia>

#endif
#ifdef ffmpegdevice

#include "ffmpegutil.h"

#endif

QList<QRect> DeviceInfoHelper::getScreenRects() {
    QList<QRect> rects;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    int screenCount = qApp->screens().count();
    QList<QScreen *> screens = qApp->screens();
    for (int i = 0; i < screenCount; ++i) {
        QScreen *screen = screens.at(i);
        //需要根据缩放比来重新调整宽高
        qreal ratio = screen->devicePixelRatio();
        QRect rect = screen->geometry();
        rect.setWidth(rect.width() * ratio);
        rect.setHeight(rect.height() * ratio);
        rects << rect;
    }
#else
    int screenCount = qApp->desktop()->screenCount();
    QDesktopWidget *desk = qApp->desktop();
    for (int i = 0; i < screenCount; ++i) {
        QRect rect = desk->screenGeometry(i);
        rects << rect;
    }
#endif
    return rects;
}

QRect DeviceInfoHelper::getScreenRect(int screenIndex) {
    //指定了屏幕索引则取指定的(没有指定则取当前鼠标所在屏幕)
    QRect rect;
    QList<QRect> rects = DeviceInfoHelper::getScreenRects();
    if (screenIndex >= 0 && screenIndex < rects.count()) {
        rect = rects.at(screenIndex);
    } else {
        //当前屏幕区域包含当前鼠标所在坐标则说明是当前屏幕
        QPoint pos = QCursor::pos();
                foreach (QRect r, rects) {
                if (r.contains(pos)) {
                    rect = r;
                    break;
                }
            }
    }
    return rect;
}

QStringList DeviceInfoHelper::getScreenInfo() {
    QStringList infos;
    QList<QRect> rects = DeviceInfoHelper::getScreenRects();
    for (int i = 0; i < rects.count(); ++i) {
        QRect rect = rects.at(i);
        infos << QString("DISPLAY%1 (%2 x %3)").arg(i).arg(rect.width()).arg(rect.height());
    }
    return infos;
}

QString DeviceInfoHelper::getScreenUrl(const QString &url) {
    QString device = url;
    int index = device.mid(7, 1).toInt();
    device.replace("(", "");
    device.replace(")", "");
    device.replace("x", "");
    QStringList list = device.split(" ");
    if (list.count() == 4) {
#if defined(Q_OS_WIN)
        QString flag = "desktop";
#elif defined(Q_OS_LINUX)
        QString flag = ":0.0";
#elif defined(Q_OS_MAC)
        QString flag = "0:0";
#endif
        device = QString("screen=%1|%2x%3|25|||%4").arg(flag).arg(list.at(1)).arg(list.at(3)).arg(index);
    }
    return device;
}

QString DeviceInfoHelper::getResolution(int width, int height) {
    //取偶数(虚拟机中很可能是奇数的分辨率)
    if (width % 2 != 0) {
        width--;
    }

    if (height % 2 != 0) {
        height--;
    }

    return QString("%1x%2").arg(width).arg(height);
}

QString DeviceInfoHelper::getResolution(const QString &resolution) {
    QStringList sizes = DeviceInfoHelper::getSizes(resolution);
    return getResolution(sizes.at(0).toInt(), sizes.at(1).toInt());
}

QStringList DeviceInfoHelper::getSizes(const QString &size) {
    QStringList sizes;
    if (size.contains("*")) {
        sizes = size.split("*");
    } else if (size.contains("x")) {
        sizes = size.split("x");
    }
    return sizes;
}

void DeviceInfoHelper::checkRect(int screenIndex, QString &resolution, int &offsetX, int &offsetY) {
    QRect rect = DeviceInfoHelper::getScreenRect(screenIndex);
    if (resolution == "0x0") {
        resolution = DeviceInfoHelper::getResolution(rect.width(), rect.height());
    } else {
        resolution = DeviceInfoHelper::getResolution(resolution);
    }

    //偏移值必须小于分辨率否则重置
    if (offsetX > rect.width()) {
        offsetX = 0;
    }
    if (offsetY > rect.height()) {
        offsetY = 0;
    }

    //判断设定的偏移值加上设定的分辨率是否超出了真实的分辨率
    QStringList sizes = DeviceInfoHelper::getSizes(resolution);
    int width = sizes.at(0).toInt();
    int height = sizes.at(1).toInt();

    if (offsetX + width > rect.width()) {
        width = rect.width() - offsetX;
    }
    if (offsetY + height > rect.height()) {
        height = rect.height() - offsetY;
    }

    //如果超出了分辨率则重新设置采集的分辨率
    resolution = DeviceInfoHelper::getResolution(width, height);

    //多个屏幕需要加上屏幕起始坐标
    if (offsetX == 0) {
        offsetX = rect.x();
    } else {
        offsetX += rect.x();
    }
    if (offsetY == 0) {
        offsetY = rect.y();
    } else {
        offsetY += rect.y();
    }

    //qDebug() << TIMEMS << screenIndex << offsetX << offsetY << resolution;
}

QStringList DeviceInfoHelper::getAudioDevices() {
    QStringList names;
#ifdef multimedia
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    QList<QAudioDevice> devices = QMediaDevices::audioInputs();
            foreach (QAudioDevice device, devices) {
            names << device.description();
        }
#else
    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach (QAudioDeviceInfo device, devices) {
        //部分版本会重复比如Qt5.9会输出两次/需要去重
        QString name = device.deviceName();
        if (!names.contains(name)) {
            names << name;
        }
    }
#endif
#endif
    return names;
}

QStringList DeviceInfoHelper::getVideoDevices() {
    QStringList names;
#ifdef multimedia
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
            foreach (QCameraDevice camera, cameras) {
            names << camera.description();
        }
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (QCameraInfo camera, cameras) {
        names << camera.description();
    }
#endif
#endif
    return names;
}

void DeviceInfoHelper::getInputDevices(QStringList &audioDevices, QStringList &videoDevices, bool screen) {
    //优先用Qt自带的方式获取
    audioDevices << DeviceInfoHelper::getAudioDevices();
    videoDevices << DeviceInfoHelper::getVideoDevices();

    //这里假设如果存在虚拟屏幕则假设虚拟声卡
    if (!audioDevices.contains("virtual-audio-capturer") && videoDevices.contains("screen-capture-recorder")) {
        audioDevices << "virtual-audio-capturer";
    }
    //上面没有获取到则用ffmpeg内置函数再获取
    FFmpegUtil::getInputDevices(false, audioDevices);
    FFmpegUtil::getInputDevices(true, videoDevices);

    //将屏幕信息加到后面
    if (screen) {
        videoDevices << DeviceInfoHelper::getScreenInfo();
    }
}

QString DeviceInfoHelper::getDeviceUrl(const QString &audioDevice, const QString &videoDevice) {
    QString url;
    if (audioDevice.isEmpty() && videoDevice.isEmpty()) {
        return url;
    }

    //优先添加桌面采集
    if (videoDevice.startsWith("DISPLAY")) {
        url = DeviceInfoHelper::getScreenUrl(videoDevice);
        return url;
    }

    QString audioName = (audioDevice == "none" ? "" : ("audio=" + audioDevice));
    QString videoName = (videoDevice == "none" ? "" : ("video=" + videoDevice));
    if (audioName.isEmpty() && videoName.isEmpty()) {
        return url;
    }

    QStringList list;
    if (!videoName.isEmpty()) {
        list << videoName;
    }
    if (!audioName.isEmpty()) {
        list << audioName;
    }

    url = list.join(":");
    return url;
}

void DeviceInfoHelper::setPara(const QStringList &list, int count, int index, int &para) {
    if (count > index && !list.at(index).isEmpty()) {
        para = list.at(index).toInt();
    }
}

void DeviceInfoHelper::setPara(const QStringList &list, int count, int index, float &para) {
    if (count > index && !list.at(index).isEmpty()) {
        para = list.at(index).toFloat();
    }
}

void DeviceInfoHelper::setPara(const QStringList &list, int count, int index, QString &para) {
    if (count > index && !list.at(index).isEmpty()) {
        para = list.at(index);
    }
}

void DeviceInfoHelper::getDevicePara(const QString &url, QString &resolution, int &frameRate, QString &codecName) {
    int offsetX, offsetY;
    QString encodeScale;
    DeviceInfoHelper::getDevicePara(url, resolution, frameRate, codecName, offsetX, offsetY, encodeScale);
}

void DeviceInfoHelper::getDevicePara(const QString &url, QString &resolution, int &frameRate, QString &codecName,
                                     int &offsetX, int &offsetY, QString &encodeScale) {
    //无论是否带分隔符第一个约定是设备名称
    QStringList list = url.split("|");
    int count = list.count();

    //默认不指定屏幕索引
    int screenIndex = -1;
    //判断是否是屏幕采集
    bool isScreen = url.startsWith("screen=");

    //带分隔符说明还指定了分辨率或帧率
    if (count > 1) {
        QStringList sizes = DeviceInfoHelper::getSizes(list.at(1));
        if (sizes.count() == 2) {
            int width = sizes.at(0).toInt();
            int height = sizes.at(1).toInt();
            resolution = QString("%1x%2").arg(width).arg(height);
        } else {
            resolution = "0x0";
        }

        //第三个参数是帧率
        DeviceInfoHelper::setPara(list, count, 2, frameRate);

        //取出其他几个参数
        if (isScreen) {
            //XY坐标偏移值
            DeviceInfoHelper::setPara(list, count, 3, offsetX);
            DeviceInfoHelper::setPara(list, count, 4, offsetY);
            //屏幕索引
            DeviceInfoHelper::setPara(list, count, 5, screenIndex);
            //视频缩放
            DeviceInfoHelper::setPara(list, count, 6, encodeScale);

            //约定屏幕索引255则取用户自定义的区域
            if (screenIndex != 255) {
                DeviceInfoHelper::checkRect(screenIndex, resolution, offsetX, offsetY);
            }
        } else {
            //指定解码器
            DeviceInfoHelper::setPara(list, count, 3, codecName);
            //视频缩放
            DeviceInfoHelper::setPara(list, count, 4, encodeScale);
        }
    }

    //没有设置分辨率则重新处理
    if (resolution == "0x0") {
        if (!isScreen) {
            resolution = "640x480";
        } else {
            DeviceInfoHelper::checkRect(screenIndex, resolution, offsetX, offsetY);
        }
    }

    //采用第三方录屏只能主屏幕/不能设置分辨率/而且不能指定区域/优点就是效率高而且光标不闪烁
    if (url.contains("video=screen-capture-recorder")) {
        resolution = "0x0";
        frameRate = (frameRate > 30 ? 30 : frameRate);
        codecName = "";
    }
}

void DeviceInfoHelper::getNormalPara(const QString &url, QString &transport, int &decodeType, int &encodeVideo,
                                     int &encodeVideoFps, float &encodeVideoRatio, QString &encodeVideoScale) {
    QStringList list = url.split("|");
    int count = list.count();

    //通信协议
    DeviceInfoHelper::setPara(list, count, 1, transport);
    //解码策略
    DeviceInfoHelper::setPara(list, count, 2, decodeType);
    //编码视频格式
    DeviceInfoHelper::setPara(list, count, 3, encodeVideo);
    //编码视频帧率
    DeviceInfoHelper::setPara(list, count, 4, encodeVideoFps);
    //视频压缩比率
    DeviceInfoHelper::setPara(list, count, 5, encodeVideoRatio);
    //视频缩放比例
    DeviceInfoHelper::setPara(list, count, 6, encodeVideoScale);
}
