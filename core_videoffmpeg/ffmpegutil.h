#pragma once

#include "ffmpeghelper.h"

class FFmpegUtil
{
public:
    //打印输出编码解码信息 https://blog.csdn.net/xu13879531489/article/details/80703465
    static void debugCodec();
    //查看硬解码 ffmpeg -hwaccels
    static void debugHardware();
    static QStringList getHardware();

    //打印编解码器和流相关参数
    static void debugPara(AVCodecContext *codecCtx);
    static void debugPara(AVStream *stream);

    //打印设备列表和参数 type: vfwcap dshow v4l2 avfoundation
    static void showDevice();
    static void showDevice(const char *flag);
    static void showDeviceOption(const QString &url);
    static void showDeviceOption(const char *flag, const QString &device);

    //获取输入输出设备名称集合
#ifdef ffmpegdevice
    static void getInputDevices(bool video, QStringList &devices);
    static QStringList getInputDevices(bool video);
    static QStringList getDeviceNames(bool input, bool video);
    static QStringList getDeviceNames(AVDeviceInfoList *devices, bool video);
#endif

    //视频帧旋转
    static void rotateFrame(int rotate, AVFrame *frameSrc, AVFrame *frameDst);

    //获取文件时长
    static qint64 getDuration(const QString &fileName, quint64 *useTime = NULL);
    //判断是否含有B帧
    static bool hasB(const QString &fileName, int maxFrame = 10);

    //2进制字符串转10进制
    static int binToDecimal(const QString &bin);
    //10进制转2进制字符串
    static QString decimalToBin(int decimal);
    //两个字节转int数据
    static int dataToInt(quint8 data1, quint8 data2);
    //解析sps/pps
    static void getExtraData(AVCodecContext *codecCtx);

    //测试函数
    static void test();

    //数据转字符串
    static QString dataToString(char *data, qint64 len);
    static QStringList dataToStringList(char *data, qint64 len);
};

