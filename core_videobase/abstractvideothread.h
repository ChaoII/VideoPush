﻿#pragma once

#include "widgethead.h"

class SaveVideo;

class SaveAudio;

class FFmpegSave;

class AbstractVideoThread : public QThread {
Q_OBJECT

public:
    //打印线程消息类别(0-不打印 1-完整地址 2-IP地址)
    static int debugInfo;

    explicit AbstractVideoThread(QObject *parent = nullptr);

    ~AbstractVideoThread() override;

protected:
    //设置视频控件尺寸
    virtual void setGeometry();

    //初始化滤镜
    virtual void initFilter();

protected:
    //数据锁
    QMutex mutex;
    //停止线程标志位
    volatile bool stopped;
    //打开是否成功
    volatile bool isOk;
    //暂停采集标志位
    volatile bool isPause;
    //开始截图标志位
    volatile bool isSnap;
    //正在录制标志位
    volatile bool isRecord;


    //设备地址
    QString ip_;
    //唯一标识
    QString flag_;

    //错误计数
    int errorCount_ = 0;
    //最后的播放时间
    QDateTime lastTime_ = QDateTime::currentDateTime();
    //计时器用来统计用时
    QElapsedTimer timer_;
    //句柄窗体
    QWidget *hwndWidget_;

    //视频显示模式
    VideoMode videoMode_ = VideoMode_Hwnd;
    //视频分辨率字符串
    QString videoSize_ = "0x0";
    //视频画面宽度
    int videoWidth_ = 0;
    //视频画面高度
    int videoHeight_ = 0;
    //视频画面帧率
    qreal frameRate_;
    //视频旋转角度
    int rotate_ = -1;

    //音频采样率
    int sampleRate_ = 8000;
    //音频通道数
    int channelCount_ = 1;
    //音频品质
    int profile_ = 1;
    //只有音频
    bool onlyAudio_ = false;

    //明亮度
    int brightness_ = 0;
    //对比度
    int contrast_ = 0;
    //色彩度
    int hue_ = 0;
    //饱和度
    int saturation_ = 0;

    //文件格式
    QString formatName_;
    //视频解码器名称
    QString videoCodecName_;
    //音频解码器名称
    QString audioCodecName_;

    //截图文件名称
    QString snapName_;
    //录制文件名称
    QString fileName_;

    //查找人脸区域
    bool findFaceRect_ = false;
    //查找相似度最高人脸
    bool findFaceOne_ = false;

    //保存视频文件类型
    SaveVideoType saveVideoType_ = SaveVideoType_None;
    //保存音频文件类型
    SaveAudioType saveAudioType_ = SaveAudioType_None;


    //编码播放速度
    double encodeSpeed_ = 1;
    //音频编码格式
    EncodeAudio encodeAudio_ = EncodeAudio_Auto;
    //视频编码格式
    EncodeVideo encodeVideo_ = EncodeVideo_None;
    //视频编码帧率
    int encodeVideoFps_ = 0;
    //视频压缩比率
    float encodeVideoRatio_ = 1;
    //视频缩放比例
    QString encodeVideoScale_ = "1";

    //保存视频到文件
    SaveVideo *saveVideo_;
    //保存音频到文件
    SaveAudio *saveAudio_;
    //保存音视频到文件
    FFmpegSave *saveFile_;

    //标签信息集合
    QList<OsdInfo> listOsd_;
    //图形信息集合
    QList<GraphInfo> listGraph_;

public:
    bool getIsOk() const;

    bool getIsPause() const;

    bool getIsSnap() const;

    bool getIsRecord() const;

public:
    //主动更新时间(一般事件回调中设置)
    void updateTime();

    //获取计时器
    QElapsedTimer *getTimer();

    //获取播放地址对应IP地址和唯一标识
    QString getIP() const;

    QString getFlag() const;

    //获取和设置视频显示模式
    VideoMode getVideoMode() const;

    void setVideoMode(const VideoMode &videoMode);

    //获取和设置画面尺寸 支持自动转换 640*480 640x480
    int getVideoWidth() const;

    int getVideoHeight() const;

    void setVideoSize(const QString &videoSize);

    //校验尺寸是否发生变化
    void checkVideoSize(int width, int height);

    //将子窗体插入到句柄控件布局
    void addWidget(QWidget *widget);

    //获取和设置采集帧率
    qreal getFrameRate() const;

    void setFrameRate(qreal frameRate);

    //获取和设置旋转角度
    int getRotate() const;

    void setRotate(int rotate);

    //获取封装格式
    QString getFormatName() const;

    //获取和设置视频解码器
    QString getVideoCodecName() const;

    void setVideoCodecName(const QString &videoCodecName);

    //获取和设置音频解码器
    QString getAudioCodecName() const;

    void setAudioCodecName(const QString &audioCodecName);

    //目录不存在则新建
    void checkPath(const QString &fileName);

    //获取和设置截图文件名称
    QString getSnapName() const;

    void setSnapName(const QString &snapName);

    //获取和设置保存文件名称
    QString getFileName() const;

    void setFileName(const QString &fileName);

    //设置查找人脸区域
    void setFindFaceRect(bool findFaceRect);

    //设置查找相似度最高人脸
    void setFindFaceOne(bool findFaceOne);

    //获取和设置保存视频文件类型
    SaveVideoType getSaveVideoType() const;

    void setSaveVideoType(const SaveVideoType &saveVideoType);

    //获取和设置保存音频文件类型
    SaveAudioType getSaveAudioType() const;

    void setSaveAudioType(const SaveAudioType &saveAudioType);

    //获取和设置编码播放速度
    double getEncodeSpeed() const;

    void setEncodeSpeed(double encodeSpeed);

    //获取和设置编码音频格式
    EncodeAudio getEncodeAudio() const;

    void setEncodeAudio(const EncodeAudio &encodeAudio);

    //获取和设置编码视频格式
    EncodeVideo getEncodeVideo() const;

    void setEncodeVideo(const EncodeVideo &encodeVideo);

    //获取和设置视频编码帧率
    int getEncodeVideoFps() const;

    void setEncodeVideoFps(int encodeVideoFps);

    //获取和设置视频压缩比率
    float getEncodeVideoRatio() const;

    void setEncodeVideoRatio(float encodeVideoRatio);

    //获取和设置视频缩放比例
    QString getEncodeVideoScale() const;

    void setEncodeVideoScale(const QString &encodeVideoScale);

public slots:

    //设置唯一标识(用来打印输出方便区分)
    virtual void setFlag(const QString &flag);

    //统一格式打印信息
    void debug(const QString &head, const QString &msg, const QString &url);

    //主动设置图片(一般事件回调中设置)
    void setImage(const QImage &image);

    //主动设置rgb图片数据(一般事件回调中设置)
    void setRgb(int width, int height, quint8 *dataRGB, int type);

    //主动设置yuv图片数据(一般事件回调中设置)
    void setYuv(int width, int height, quint8 *dataY, quint8 *dataU, quint8 *dataV);

public slots:

    //开始播放
    virtual void play();

    //停止播放
    virtual void stop();

    //暂停播放
    virtual void pause();

    //继续播放
    virtual void next();

    //抓拍截图
    virtual void snap(const QString &snapName);

    //截图完成
    virtual void snapFinish(const QImage &image);

    //获取图片
    virtual QImage getImage();

    //开始录制
    virtual void recordStart(const QString &fileName);

    //暂停录制
    virtual void recordPause();

    //停止录制
    virtual void recordStop();

    //写入视频数据到文件
    void writeVideoData(int width, int height, quint8 *dataY, quint8 *dataU, quint8 *dataV);

    //写入音频数据到文件
    void writeAudioData(const char *data, qint64 len);

    void writeAudioData(const QByteArray &data);

    //设置标签信息集合
    virtual void setOsdInfo(const QList<OsdInfo> &listOsd);

    //设置图形信息集合
    virtual void setGraphInfo(const QList<GraphInfo> &listGraph);

signals:

    //播放成功
    void receivePlayStart(int time);

    //播放结束
    void receivePlayFinish();

    //播放失败
    void receivePlayError(int error);

    //收到一张图片
    void receiveImage(const QImage &image, int time);

    //抓拍一张图片
    void snapImage(const QImage &image, const QString &snapName);

    //视频尺寸变化
    void receiveSizeChanged();

    //录制状态变化
    void recorderStateChanged(const RecorderState &state, const QString &file);

    //收到一帧rgb视频数据
    void receiveFrame(int width, int height, quint8 *dataRGB, int type);

    //收到一帧yuv视频数据
    void receiveFrame(int width, int height, quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 lineSizeY,
                      quint32 lineSizeU, quint32 lineSizeV);

    //收到一帧nv12视频数据
    void receiveFrame(int width, int height, quint8 *dataY, quint8 *dataUV, quint32 lineSizeY, quint32 lineSizeUV);

    //音量大小
    void receiveVolume(int volume);

    //静音状态
    void receiveMuted(bool muted);

    //音频数据振幅
    void receiveLevel(qreal leftLevel, qreal rightLevel);

    //视频实时码率
    void receiveKbps(qreal kbps, int frameRate);
};

