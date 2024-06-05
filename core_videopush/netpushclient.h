#pragma once

#include "widgetstruct.h"
#include "core_videoffmpeg/ffmpegsave.h"
#include "videopushhelper.h"
#include "core_videoffmpeg/ffmpeghelper.h"

class NetPushClient : public QObject {
Q_OBJECT

public:
    //是否检查B帧
    static bool checkB;
    //录像文件策略
    static int recordType;
    //视频编码格式
    static int encodeVideo;
    //视频压缩比率
    static float encodeVideoRatio;
    //视频缩放比例
    static QString encodeVideoScale;
    //测试文字水印
    static QList<OsdInfo> osds;

    explicit NetPushClient(QObject *parent = nullptr);

    ~NetPushClient();

private:
    //播放地址
    QString mediaUrl;
    //推流地址
    QString pushUrl;
    //视频采集线程
    FFmpegThread *ffmpegThread;
    //视频保存线程
    FFmpegSave *ffmpegSave;

    //录制文件切断定时器
    QTimer *timerRecord;
    //最后启动录制的时间
    QDateTime recordTime;

public:
    //获取播放地址
    QString getMediaUrl();

    //获取推流地址
    QString getPushUrl();

    //获取采集线程
    FFmpegThread *getVideoThread();

private slots:

    //定时器判断录制
    void checkRecord();

    void record();

    //播放成功
    void receivePlayStart(int time);

    //收到一帧数据(一般用于多路存储或推流)
    void receivePacket(AVPacket *packet);

    //录制状态变化
    void recorderStateChanged(const RecorderState &state, const QString &file);

    //保存成功
    void receiveSaveStart();

    //保存结束
    void receiveSaveFinish();

    //保存失败
    void receiveSaveError(int error);

public slots:

    //设置播放地址
    void setMediaUrl(const QString &mediaUrl);

    //设置推流地址
    void setPushUrl(const QString &pushUrl);

    //开始推流
    void start();

    //停止推流
    void stop();

signals:

    //推流开始
    void pushStart(const QString &mediaUrl, int width, int height, int videoStatus, int audioStatus, bool start);

    //推流状态变化
    void pushChanged(const QString &mediaUrl, int state);
};

