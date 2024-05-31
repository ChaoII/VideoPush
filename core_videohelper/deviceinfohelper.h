#pragma once

#include <QObject>
#include <QRect>

class DeviceInfoHelper
{
public:
    //获取当前屏幕区域
    static QList<QRect> getScreenRects();
    static QRect getScreenRect(int screenIndex = -1);

    //获取本地屏幕信息
    static QStringList getScreenInfo();
    static QString getScreenUrl(const QString &url);

    //传入宽高输出分辨率字符串(矫正奇数)
    static QString getResolution(int width, int height);
    static QString getResolution(const QString &resolution);

    //分辨率字符串分割 640x480/640*480
    static QStringList getSizes(const QString &size);

    //根据屏幕区域校验参数
    static void checkRect(int screenIndex, QString &resolution, int &offsetX, int &offsetY);

    //获取本地视音频输入设备名称
    static QStringList getAudioDevices();
    static QStringList getVideoDevices();
    static void getInputDevices(QStringList &audioDevices, QStringList &videoDevices, bool screen = true);
    static QString getDeviceUrl(const QString &audioDevice, const QString &videoDevice);

    //从字符串数组设置参数
    static void setPara(const QStringList &list, int count, int index, int &para);
    static void setPara(const QStringList &list, int count, int index, float &para);
    static void setPara(const QStringList &list, int count, int index, QString &para);

    //通用函数取出本地设备带分辨率和帧率等信息
    static void getDevicePara(const QString &url, QString &resolution, int &frameRate, QString &codecName);
    static void getDevicePara(const QString &url, QString &resolution, int &frameRate, QString &codecName, int &offsetX, int &offsetY, QString &encodeScale);
    static void getNormalPara(const QString &url, QString &transport, int &decodeType, int &encodeVideo, int &encodeVideoFps, float &encodeVideoRatio, QString &encodeVideoScale);
};

