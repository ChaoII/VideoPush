#include "netpushclient.h"
#include "ffmpegthread.h"
#include "ffmpegsave.h"
#include "widgethelper.h"
#include "videohelper.h"

bool NetPushClient::checkB = false;
int NetPushClient::recordType = 0;
int NetPushClient::encodeVideo = 0;
float NetPushClient::encodeVideoRatio = 1;
QString NetPushClient::encodeVideoScale = "1";

NetPushClient::NetPushClient(QObject *parent) : QObject(parent)
{
    ffmpegThread = NULL;
    ffmpegSave = NULL;

    //定时器控制多久录制一个文件
    timerRecord = new QTimer(this);
    timerRecord->setInterval(1000);
    connect(timerRecord, SIGNAL(timeout()), this, SLOT(checkRecord()));
}

NetPushClient::~NetPushClient()
{
    this->stop();
}

QString NetPushClient::getMediaUrl()
{
    return this->mediaUrl;
}

QString NetPushClient::getPushUrl()
{
    return this->pushUrl;
}

FFmpegThread *NetPushClient::getVideoThread()
{
    return this->ffmpegThread;
}

void NetPushClient::checkRecord()
{
    //保存策略说明
    //0: 不保存
    //1: 每30分钟保存一个文件/以本地时间计时
    //2: 每60分钟保存一个文件/以本地时间计时
    //3: 其他数字表示多长时长保存一次/以开始保存计时/单位分钟

    QDateTime now = QDateTime::currentDateTime();
    QTime time = now.time();
    int min = time.minute();
    int sec = time.second();
    qint64 offset = recordTime.msecsTo(now);

    //定时器未必很准确/所以下面以时间差作为判断
    bool ok = false;
    if (recordType == 1) {
        ok = (min == 30 && (sec >= 0 || sec <= 2));
    } else if (recordType == 2) {
        ok = (min == 0 && (sec >= 0 || sec <= 2));
    } else {
        ok = (offset >= (recordType * 60 * 1000));
    }

    //保证最小录制时长
    if (ok && offset >= 5000) {
        this->record();
    }
}

void NetPushClient::record()
{
    if (ffmpegSave) {
        //取出推流码
        QString flag = pushUrl.split("/").last();
        //文件名不能包含特殊字符/需要替换成固定字母
        QString pattern("[\\\\/:|*?\"<>]|[cC][oO][mM][1-9]|[lL][pP][tT][1-9]|[cC][oO][nM]|[pP][rR][nN]|[aA][uU][xX]|[nN][uU][lL]");
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        QRegularExpression rx(pattern);
#else
        QRegExp rx(pattern);
#endif
        flag.replace(rx, "X");

        //文件名加上时间结尾
        QString path = QString("%1/video/%2").arg(qApp->applicationDirPath()).arg(QDATE);
        QString name = QString("%1/%2_%3.mp4").arg(path).arg(flag).arg(STRDATETIME);

        //目录不存在则新建
        QDir dir(path);
        if (!dir.exists()) {
            dir.mkpath(path);
        }

        //先停止再打开重新录制
        ffmpegSave->stop();
        ffmpegSave->open(name);
        recordTime = QDateTime::currentDateTime();
    }
}

void NetPushClient::receivePlayStart(int time)
{
    //演示添加OSD后推流
#ifdef betaversion
    int height = ffmpegThread->getVideoHeight();
    QList<OsdInfo> osds = WidgetHelper::getTestOsd(height);
    ffmpegThread->setOsdInfo(osds);
#endif

    //打开后才能启动录像
    ffmpegThread->recordStart(pushUrl);

    //推流以外还单独存储
    if (!ffmpegSave && recordType > 0) {
        //源头保存没成功就不用继续
        FFmpegSave *saveFile = ffmpegThread->getSaveFile();
        if (!saveFile->getIsOk()) {
            return;
        }

        ffmpegSave = new FFmpegSave(this);
        //重新编码过的则取视频保存类的对象
        AVStream *videoStreamIn = saveFile->getVideoEncode() ? saveFile->getVideoStream() : ffmpegThread->getVideoStream();
        AVStream *audioStreamIn = saveFile->getAudioEncode() ? saveFile->getAudioStream() : ffmpegThread->getAudioStream();
        ffmpegSave->setSavePara(ffmpegThread->getMediaType(), SaveVideoType_Mp4, videoStreamIn, audioStreamIn);
        this->record();
        timerRecord->start();
    }
}

void NetPushClient::receivePacket(AVPacket *packet)
{
    if (ffmpegSave && ffmpegSave->getIsOk()) {
        ffmpegSave->writePacket2(packet);
    }

    FFmpegHelper::freePacket(packet);
}

void NetPushClient::recorderStateChanged(const RecorderState &state, const QString &file)
{
    int width = 0;
    int height = 0;
    int videoStatus = 0;
    int audioStatus = 0;
    if (ffmpegThread) {
        width = ffmpegThread->getVideoWidth();
        height = ffmpegThread->getVideoHeight();
        FFmpegSave *saveFile = ffmpegThread->getSaveFile();
        if (saveFile->getIsOk()) {
            if (saveFile->getVideoIndexIn() >= 0) {
                if (saveFile->getVideoIndexOut() >= 0) {
                    videoStatus = (saveFile->getVideoEncode() ? 3 : 2);
                } else {
                    videoStatus = 1;
                }
            }
            if (saveFile->getAudioIndexIn() >= 0) {
                if (saveFile->getAudioIndexOut() >= 0) {
                    audioStatus = (saveFile->getAudioEncode() ? 3 : 2);
                } else {
                    audioStatus = 1;
                }
            }
        }
    }

    //只有处于录制中才表示正常推流开始
    bool start = (state == RecorderState_Recording);
    emit pushStart(mediaUrl, width, height, videoStatus, audioStatus, start);
}

void NetPushClient::receiveSaveStart()
{
    emit pushChanged(mediaUrl, 0);
}

void NetPushClient::receiveSaveFinsh()
{
    emit pushChanged(mediaUrl, 1);
}

void NetPushClient::receiveSaveError(int error)
{
    emit pushChanged(mediaUrl, 2);
}

void NetPushClient::setMediaUrl(const QString &mediaUrl)
{
    this->mediaUrl = mediaUrl;
}

void NetPushClient::setPushUrl(const QString &pushUrl)
{
    this->pushUrl = pushUrl;
}

void NetPushClient::start()
{
    if (ffmpegThread || mediaUrl.isEmpty() || pushUrl.isEmpty()) {
        return;
    }

    //实例化视频采集线程
    ffmpegThread = new FFmpegThread;
    //关联播放开始信号用来启动推流
    connect(ffmpegThread, SIGNAL(receivePlayStart(int)), this, SLOT(receivePlayStart(int)));
    //关联录制信号变化用来判断是否推流成功
    connect(ffmpegThread, SIGNAL(recorderStateChanged(RecorderState, QString)), this, SLOT(recorderStateChanged(RecorderState, QString)));
    //设置播放地址
    ffmpegThread->setMediaUrl(mediaUrl);
    //设置解码内核
    ffmpegThread->setVideoCore(VideoCore_FFmpeg);

    //设置视频模式
#ifdef openglx
    ffmpegThread->setVideoMode(VideoMode_Opengl);
#else
    ffmpegThread->setVideoMode(VideoMode_Painter);
#endif

    //设置通信协议(如果是rtsp视频流建议设置tcp)
    //ffmpegThread->setTransport("tcp");
    //设置硬解码(和推流无关/只是为了加速显示/推流只和硬编码有关)
    //ffmpegThread->setHardware("dxva2");
    //设置缓存大小(如果分辨率帧率码流很大需要自行加大缓存)
    ffmpegThread->setCaching(8192000);
    //设置解码策略(推流的地址再拉流建议开启最快速度)
    //ffmpegThread->setDecodeType(DecodeType_Fastest);

    //设置读取超时时间超时后会自动重连
    ffmpegThread->setReadTimeout(5 * 1000);
    //设置连接超时时间(0表示一直连)
    ffmpegThread->setConnectTimeout(0);
    //设置重复播放相当于循环推流
    ffmpegThread->setPlayRepeat(true);
    //设置默认不播放音频(界面上切换到哪一路就开启)
    ffmpegThread->setPlayAudio(false);
    //设置默认不预览视频(界面上切换到哪一路就开启)
    ffmpegThread->setPushPreview(false);

    //设置保存视频类将数据包信号发出来用于保存文件
    FFmpegSave *saveFile = ffmpegThread->getSaveFile();
    saveFile->setProperty("checkB", checkB);
    saveFile->setSendPacket(recordType > 0, false);
    connect(saveFile, SIGNAL(receivePacket(AVPacket *)), this, SLOT(receivePacket(AVPacket *)));
    connect(saveFile, SIGNAL(receiveSaveStart()), this, SLOT(receiveSaveStart()));
    connect(saveFile, SIGNAL(receiveSaveFinsh()), this, SLOT(receiveSaveFinsh()));
    connect(saveFile, SIGNAL(receiveSaveError(int)), this, SLOT(receiveSaveError(int)));

    //如果是本地设备或者桌面录屏要取出其他参数
    VideoHelper::initVideoPara(ffmpegThread, mediaUrl, encodeVideoScale);

    //设置视频编码格式/视频压缩比率/视频缩放比例
    ffmpegThread->setEncodeVideo((EncodeVideo)encodeVideo);
    ffmpegThread->setEncodeVideoRatio(encodeVideoRatio);
    ffmpegThread->setEncodeVideoScale(encodeVideoScale);

    //启动播放
    ffmpegThread->play();
}

void NetPushClient::stop()
{
    //停止推流和采集并彻底释放对象
    if (ffmpegThread) {
        ffmpegThread->recordStop();
        ffmpegThread->stop();
        ffmpegThread->deleteLater();
        ffmpegThread = NULL;
    }

    //停止录制
    if (ffmpegSave) {
        timerRecord->stop();
        ffmpegSave->stop();
        ffmpegSave->deleteLater();
        ffmpegSave = NULL;
    }
}
