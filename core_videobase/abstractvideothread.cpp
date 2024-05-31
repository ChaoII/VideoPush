#include "abstractvideothread.h"
#include "deviceinfohelper.h"
#include "urlhelper.h"

#ifdef videosave
#include "savevideo.h"
#include "saveaudio.h"
#endif

#ifdef ffmpeg
#include "ffmpegsave.h"
#endif

int AbstractVideoThread::debugInfo = 2;
AbstractVideoThread::AbstractVideoThread(QObject *parent) : QThread(parent)
{
    //注册数据类型
    qRegisterMetaType<RecorderState>("RecorderState");
    qRegisterMetaType<QList<int> >("QList<int>");

    stopped = false;
    isOk = false;
    isPause = false;
    isSnap = false;
    isRecord = false;

    hwndWidget_ = (QWidget *)parent;

#ifdef videosave
    saveVideo_ = new SaveVideo(this);
    saveAudio_ = new SaveAudio(this);
#endif
#ifdef ffmpeg
    saveFile_ = new FFmpegSave(this);
#endif
}

AbstractVideoThread::~AbstractVideoThread()
{
    this->stop();
}

void AbstractVideoThread::setGeometry()
{

}

void AbstractVideoThread::initFilter()
{

}

bool AbstractVideoThread::getIsOk() const
{
    return this->isOk;
}

bool AbstractVideoThread::getIsPause() const
{
    return this->isPause;
}

bool AbstractVideoThread::getIsSnap() const
{
    return this->isSnap;
}

bool AbstractVideoThread::getIsRecord() const
{
    return this->isRecord;
}

void AbstractVideoThread::updateTime()
{
    lastTime_ = QDateTime::currentDateTime();
}

QElapsedTimer *AbstractVideoThread::getTimer()
{
    return &timer_;
}

QString AbstractVideoThread::getIP() const
{
    return this->ip_;
}

QString AbstractVideoThread::getFlag() const
{
    return this->flag_;
}

VideoMode AbstractVideoThread::getVideoMode() const
{
    return this->videoMode_;
}

void AbstractVideoThread::setVideoMode(const VideoMode &videoMode)
{
    this->videoMode_ = videoMode;
}

int AbstractVideoThread::getVideoWidth() const
{
    return this->videoWidth_;
}

int AbstractVideoThread::getVideoHeight() const
{
    return this->videoHeight_;
}

void AbstractVideoThread::setVideoSize(const QString &videoSize)
{
    this->videoSize_ = videoSize;
    QStringList sizes = DeviceInfoHelper::getSizes(videoSize);
    if (sizes.count() == 2) {
        videoWidth_ = sizes.at(0).toInt();
        videoHeight_ = sizes.at(1).toInt();
    }
}

void AbstractVideoThread::checkVideoSize(int width, int height)
{
    if (width > 0 && height > 0 && videoWidth_ > 0 && videoHeight_ > 0) {
        if (videoWidth_ != width || videoHeight_ != height) {
            videoWidth_ = width;
            videoHeight_ = height;
            QMetaObject::invokeMethod(this, "receiveSizeChanged");
        }
    }
}

void AbstractVideoThread::addWidget(QWidget *widget)
{
    //必须是视频而且句柄模式才需要加入视频控件
    if (onlyAudio_ || videoMode_ != VideoMode_Hwnd) {
        return;
    }

    //有时候可能只是线程在用/没有设置窗体展示/比如只需要后台解码的情况
    if (hwndWidget_ && hwndWidget_->layout()) {
        hwndWidget_->layout()->addWidget(widget);
    }

    QMetaObject::invokeMethod(widget, "show");
}

qreal AbstractVideoThread::getFrameRate() const
{
    return this->frameRate_;
}

void AbstractVideoThread::setFrameRate(qreal frameRate)
{
    this->frameRate_ = frameRate;
}

int AbstractVideoThread::getRotate() const
{
    return this->rotate_;
}

void AbstractVideoThread::setRotate(int rotate)
{
    this->rotate_ = rotate;
}

QString AbstractVideoThread::getFormatName() const
{
    return this->formatName_;
}

QString AbstractVideoThread::getVideoCodecName() const
{
    return this->videoCodecName_;
}

void AbstractVideoThread::setVideoCodecName(const QString &videoCodecName)
{
    this->videoCodecName_ = videoCodecName.toLower();
}

QString AbstractVideoThread::getAudioCodecName() const
{
    return this->audioCodecName_;
}

void AbstractVideoThread::setAudioCodecName(const QString &audioCodecName)
{
    this->audioCodecName_ = audioCodecName.toLower();
}

void AbstractVideoThread::checkPath(const QString &fileName)
{
    //推流则不用继续
    if (fileName.startsWith("rtsp://") || fileName.startsWith("rtmp://")) {
        return;
    }

    //补全完整路径
    QString path = QFileInfo(fileName).path();
    if (path.startsWith("./")) {
        path.replace(".", "");
        path = qApp->applicationDirPath() + path;
    }

    //目录不存在则新建
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(path);
    }
}

QString AbstractVideoThread::getSnapName() const
{
    return this->snapName_;
}

void AbstractVideoThread::setSnapName(const QString &snapName)
{
    this->snapName_= snapName;
    this->checkPath(snapName);
}

QString AbstractVideoThread::getFileName() const
{
    return this->fileName_;
}

void AbstractVideoThread::setFileName(const QString &fileName)
{
    this->fileName_ = fileName;
    UrlHelper::checkPrefix(this->fileName_);
    this->checkPath(this->fileName_);
}

void AbstractVideoThread::setFindFaceRect(bool findFaceRect)
{
    this->findFaceRect_ = findFaceRect;
}

void AbstractVideoThread::setFindFaceOne(bool findFaceOne)
{
    this->findFaceOne_ = findFaceOne;
}

SaveVideoType AbstractVideoThread::getSaveVideoType() const
{
    return this->saveVideoType_;
}

void AbstractVideoThread::setSaveVideoType(const SaveVideoType &saveVideoType)
{
    this->saveVideoType_ = saveVideoType;
}

SaveAudioType AbstractVideoThread::getSaveAudioType() const
{
    return this->saveAudioType_;
}

void AbstractVideoThread::setSaveAudioType(const SaveAudioType &saveAudioType)
{
    this->saveAudioType_ = saveAudioType;
}

double AbstractVideoThread::getEncodeSpeed() const
{
    return this->encodeSpeed_;
}

void AbstractVideoThread::setEncodeSpeed(double encodeSpeed)
{
    this->encodeSpeed_= encodeSpeed;
}

EncodeAudio AbstractVideoThread::getEncodeAudio() const
{
    return this->encodeAudio_;
}

void AbstractVideoThread::setEncodeAudio(const EncodeAudio &encodeAudio)
{
    this->encodeAudio_ = encodeAudio;
}

EncodeVideo AbstractVideoThread::getEncodeVideo() const
{
    return this->encodeVideo_;
}

void AbstractVideoThread::setEncodeVideo(const EncodeVideo &encodeVideo)
{
    this->encodeVideo_ = encodeVideo;
}

int AbstractVideoThread::getEncodeVideoFps() const
{
    return this->encodeVideoFps_;
}

void AbstractVideoThread::setEncodeVideoFps(int encodeVideoFps)
{
    if (encodeVideoFps >= 1 && encodeVideoRatio_ <= 30) {
        this->encodeVideoFps_ = encodeVideoFps;
    }
}

float AbstractVideoThread::getEncodeVideoRatio() const
{
    return this->encodeVideoRatio_;
}

void AbstractVideoThread::setEncodeVideoRatio(float encodeVideoRatio)
{
    if (encodeVideoRatio >= 0.1 && encodeVideoRatio < 1) {
        this->encodeVideoRatio_ = encodeVideoRatio;
    }
}

QString AbstractVideoThread::getEncodeVideoScale() const
{
    return this->encodeVideoScale_;
}

void AbstractVideoThread::setEncodeVideoScale(const QString &encodeVideoScale)
{
    this->encodeVideoScale_ = encodeVideoScale;
}

void AbstractVideoThread::setFlag(const QString &flag)
{
    this->flag_ = flag;
#ifdef videosave
    saveVideo_->setFlag(flag);
    saveAudio_->setFlag(flag);
#endif
#ifdef ffmpeg
    saveFile_->setFlag(flag);
#endif
}

void AbstractVideoThread::debug(const QString &head, const QString &msg, const QString &url)
{
    if (debugInfo == 0) {
        return;
    }

    //如果设置了唯一标识则放在打印前面
    QString text = head;
    if (!flag_.isEmpty()) {
        text = QString("标识[%1] %2").arg(flag_).arg(text);
    }

    QString addr = url;
    if (debugInfo == 2) {
        //为空的时候获取一次即可
        if (ip_.isEmpty()) {
            //本地文件则取末尾
            if (QFile(url).exists() && !url.contains("/dev/")) {
                ip_ = url.split("/").last();
            } else {
                ip_ = UrlHelper::getUrlIP(url);
            }
        }
        addr = ip_;
    }

    if (msg.isEmpty()) {
        qDebug() << TIMEMS << QString("%1 -> 地址: %2").arg(text).arg(addr);
    } else {
        qDebug() << TIMEMS << QString("%1 -> %2 地址: %3").arg(text).arg(msg).arg(addr);
    }
}

void AbstractVideoThread::setImage(const QImage &image)
{
    this->updateTime();
    emit receiveImage(image, 0);
}

void AbstractVideoThread::setRgb(int width, int height, quint8 *dataRGB, int type)
{
    this->updateTime();
    emit receiveFrame(width, height, dataRGB, type);
}

void AbstractVideoThread::setYuv(int width, int height, quint8 *dataY, quint8 *dataU, quint8 *dataV)
{
    this->updateTime();
    emit receiveFrame(width, height, dataY, dataU, dataV, width, width / 2, width / 2);
}

void AbstractVideoThread::play()
{
    //没有运行才需要启动
    if (!this->isRunning()) {
        stopped = false;
        isPause = false;
        isSnap = false;
        this->start();
    }
}

void AbstractVideoThread::stop()
{
    //处于运行状态才可以停止
    if (this->isRunning()) {
        stopped = true;
        isPause = false;
        isSnap = false;
        this->wait();
    }
}

void AbstractVideoThread::pause()
{
    if (this->isRunning()) {
        isPause = true;
    }
}

void AbstractVideoThread::next()
{
    if (this->isRunning()) {
        isPause = false;
    }
}

void AbstractVideoThread::snap(const QString &snapName)
{
    if (this->isRunning()) {
        isSnap = true;
        this->setSnapName(snapName);
    }
}

void AbstractVideoThread::snapFinish(const QImage &image)
{
    //如果填了截图文件名称则先保存图片
    if (!snapName_.isEmpty() && !image.isNull()) {
        //取出拓展名根据拓展名保存格式
        QString suffix = snapName_.split(".").last();
        image.save(snapName_, suffix.toLatin1().constData());
    }

    //发送截图完成信号
    emit snapImage(image, snapName_);
}

QImage AbstractVideoThread::getImage()
{
    return QImage();
}

void AbstractVideoThread::recordStart(const QString &fileName)
{
#ifdef videosave
    this->setFileName(fileName);
    if (saveAudioType > 0) {
        //处于暂停阶段则切换暂停标志位(暂停后再次恢复说明又重新开始录制)
        if (saveAudio->getIsPause()) {
            saveAudio->pause();
            emit recorderStateChanged(RecorderState_Recording, saveAudio->getFileName());
        } else {
            saveAudio->setPara(saveAudioType, sampleRate, channelCount, profile);
            saveAudio->open(fileName);
            if (saveAudio->getIsOk()) {
                emit recorderStateChanged(RecorderState_Recording, saveAudio->getFileName());
            }
        }
    }

    if (saveVideoType == SaveVideoType_Yuv && !onlyAudio) {
        //处于暂停阶段则切换暂停标志位(暂停后再次恢复说明又重新开始录制)
        if (saveVideo->getIsPause()) {
            isRecord = true;
            saveVideo->pause();
            emit recorderStateChanged(RecorderState_Recording, saveVideo->getFileName());
        } else {
            saveVideo->setPara(saveVideoType, videoWidth, videoHeight, frameRate);
            saveVideo->open(fileName);
            if (saveVideo->getIsOk()) {
                isRecord = true;
                emit recorderStateChanged(RecorderState_Recording, saveVideo->getFileName());
            }
        }
    }
#endif
}

void AbstractVideoThread::recordPause()
{
#ifdef videosave
    if (saveAudioType > 0) {
        if (saveAudio->getIsOk()) {
            saveAudio->pause();
            emit recorderStateChanged(RecorderState_Paused, saveAudio->getFileName());
        }
    }

    if (saveVideoType == SaveVideoType_Yuv && !onlyAudio) {
        if (saveVideo->getIsOk()) {
            isRecord = false;
            saveVideo->pause();
            emit recorderStateChanged(RecorderState_Paused, saveVideo->getFileName());
        }
    }
#endif
}

void AbstractVideoThread::recordStop()
{
#ifdef videosave
    if (saveAudioType > 0) {
        if (saveAudio->getIsOk()) {
            saveAudio->stop();
            emit recorderStateChanged(RecorderState_Stopped, saveAudio->getFileName());
        }
    }

    if (saveVideoType == SaveVideoType_Yuv && !onlyAudio) {
        if (saveVideo->getIsOk()) {
            isRecord = false;
            saveVideo->stop();
            emit recorderStateChanged(RecorderState_Stopped, saveVideo->getFileName());
        }
    }
#endif
}

void AbstractVideoThread::writeVideoData(int width, int height, quint8 *dataY, quint8 *dataU, quint8 *dataV)
{
#ifdef videosave
    if (saveVideoType == SaveVideoType_Yuv && isRecord) {
        saveVideo->setPara(SaveVideoType_Yuv, width, height, frameRate);
        saveVideo->write(dataY, dataU, dataV);
    }
#endif
}

void AbstractVideoThread::writeAudioData(const char *data, qint64 len)
{
#ifdef videosave
    if (saveAudioType > 0 && saveAudio->getIsOk()) {
        saveAudio->write(data, len);
    }
#endif
}

void AbstractVideoThread::writeAudioData(const QByteArray &data)
{
#ifdef videosave
    if (saveAudioType > 0 && saveAudio->getIsOk()) {
        this->writeAudioData(data.constData(), data.length());
    }
#endif
}

void AbstractVideoThread::setOsdInfo(const QList<OsdInfo> &listOsd)
{
    this->listOsd_ = listOsd;
    this->initFilter();
}

void AbstractVideoThread::setGraphInfo(const QList<GraphInfo> &listGraph)
{
    this->listGraph_ = listGraph;
    this->initFilter();
}
