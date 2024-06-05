#include "videowidgetx.h"
#include "videohelper.h"
#include "videotask.h"
#include "core_videobase/bannerwidget.h"
#include "core_videobase/widgethelper.h"
#include "core_videohelper/urlhelper.h"
#include <QOverload>

VideoWidget::VideoWidget(QWidget *parent) : AbstractVideoWidget(parent) {
    videoWidth_ = 0;
    videoHeight_ = 0;
    rotate_ = -1;
    videoThread = nullptr;

    //关联按钮单击事件(对应功能可以直接处理掉)
    connect(this, &VideoWidget::sigBtnClicked, this, &VideoWidget::btnClicked);
    //关联鼠标按下用于电子放大
    connect(this, &VideoWidget::sigReceivePoint, this, &VideoWidget::receivePoint);
    //关联标签和图形信息变化
    connect(this, &VideoWidget::sigOsdChanged, this, &VideoWidget::osdChanged);
    connect(this, &VideoWidget::sigGraphChanged, this, &VideoWidget::graphChanged);
}

VideoWidget::~VideoWidget() {
    this->stop();
}

void VideoWidget::resizeEvent(QResizeEvent *e) {
    if (!videoThread) {
        AbstractVideoWidget::resizeEvent(nullptr);
        return;
    }

    //主动调用resizeEvent(nullptr)后产生的sender有对象也需要触发
    if (e || sender()) {
        //将尺寸等信息赋值给基类(基类那边需要用到这些变量)
        videoWidth_ = videoThread->getVideoWidth();
        videoHeight_ = videoThread->getVideoHeight();
        //频谱区域高度(easyplayer内核用于底部显示可视化频谱)
        if (videoPara.videoCore == VideoCore_EasyPlayer) {
            visualHeight_ = videoHeight_ * 0.1;
        }

        rotate_ = videoThread->getRotate();
        onlyAudio_ = videoThread->getOnlyAudio();
        hardware_ = videoThread->getHardware();

        //先执行父类的尺寸变化
        AbstractVideoWidget::resizeEvent(nullptr);
        videoThread->debug("窗体拉伸", QString("宽高: %1x%2 角度: %3").arg(videoWidth_).arg(videoHeight_).arg(rotate_));

        //在拉伸填充模式下可能还没获取到尺寸就会触发一次
        if (!onlyAudio_ && (videoWidth_ <= 0 || videoHeight_ <= 0)) {
            return;
        }

        //vlc核mpv内核句柄拉伸填充模式下需要主动设置拉伸比
        if (widgetPara_.videoMode == VideoMode_Hwnd) {
            if (videoPara.videoCore == VideoCore_Mpv) {
                if (widgetPara_.scaleMode == ScaleMode_Fill) {
                    this->resize2();
                }
            } else if (videoPara.videoCore == VideoCore_Vlc) {
                if (widgetPara_.scaleMode == ScaleMode_Fill) {
                    this->resize2();
                    //vlc内核视频流有时候需要再执行一次
                    if (!videoThread->getIsFile()) {
                        QTimer::singleShot(100, this, SLOT(resize2()));
                    }
                } else if (videoThread->getMediaType() == MediaType_Device) {
                    this->setAspect(videoWidth_, videoHeight_);
                }
            }
        }
    }
}

VideoPara VideoWidget::getVideoPara() const {
    return this->videoPara;
}

void VideoWidget::setVideoPara(const VideoPara &videoPara) {
    this->videoPara = videoPara;
}

QString VideoWidget::getBannerText() const {
    if (!getIsRunning()) {
        return "";
    }

    //悬浮条如果宽度不够则不显示文字信息
    int width = bannerWidget_->width();
    if (width < 200) {
        return "";
    }

    //线程已经开启并处于打开中/还未打开完成
    if (!videoThread->getIsOk()) {
        return "打开中...";
    }

    QStringList list;
    if (bannerInfo_.resolution) {
        if (videoWidth_ > 0) {
            list << QString("%1%2 x %3").arg(width > 240 ? "分辨率: " : "").arg(videoWidth_).arg(videoHeight_);
        }
    }

    if (bannerInfo_.frameRate) {
        int fps = videoThread->getFrameRate();
        if (fps > 0) {
            list << QString("帧率: %1").arg(fps);
        }
    }

    if (bannerInfo_.mediaUrl) {
        QString ip = videoThread->getIP();
        if (!ip.isEmpty() && UrlHelper::isIP(ip)) {
            list << QString("地址: %1").arg(ip);
        }
    }

    if (bannerInfo_.formatName) {
        QString name = VideoHelper::getFormatName(videoThread);
        if (!name.isEmpty()) {
            list << QString("格式: %1").arg(name);
        }
    }

    if (bannerInfo_.videoCodecName) {
        QString name = videoThread->getVideoCodecName();
        if (!name.isEmpty()) {
            list << QString("视频: %1").arg(name);
        }
    }

    if (bannerInfo_.audioCodecName) {
        QString name = videoThread->getAudioCodecName();
        if (!name.isEmpty()) {
            list << QString("音频: %1").arg(name);
        }
    }

    if (bannerInfo_.realBitRate) {
        if (kbps_ > 0) {
            list << QString("码率: %1kbps").arg((int) kbps_);
        }
    }

    if (bannerInfo_.hardware) {
        QString hardware = videoThread->getHardware();
        list << QString("硬解: %1").arg(hardware);
    }

    if (bannerInfo_.videoMode) {
        QString videoMode = "句柄";
        if (widgetPara_.videoMode == VideoMode_Opengl) {
            videoMode = "GPU";
        } else if (widgetPara_.videoMode == VideoMode_Painter) {
            videoMode = "绘制";
        }
        list << QString("模式: %1").arg(videoMode);
    }

    return list.join("  ");
}

void VideoWidget::setAudioLevel(bool audioLevel) {
    if (videoThread) {
        videoThread->setAudioLevel(audioLevel);
    }
}

void VideoWidget::setRealBitRate(bool realBitRate) {
    if (videoThread) {
        videoThread->setRealBitRate(realBitRate);
    }
}

bool VideoWidget::getIsPause() const {
    if (videoThread) {
        return videoThread->getIsPause();
    } else {
        return false;
    }
}

bool VideoWidget::getIsRecord() const {
    return bannerWidget_->getIsRecord();
}

bool VideoWidget::getIsCrop() const {
    return bannerWidget_->getIsCrop();
}

QImage VideoWidget::getImage() const {
    QImage image;
    if (videoThread) {
        image = videoThread->getImage();
    }

    return image;
}

VideoThread *VideoWidget::getVideoThread() const {
    return this->videoThread;
}

void VideoWidget::setPlayRepeat(bool playRepeat) {
    if (videoThread) {
        videoPara.playRepeat = playRepeat;
        videoThread->setPlayRepeat(playRepeat);
    }
}

void VideoWidget::setReadTimeout(int readTimeout) {
    if (videoThread) {
        videoPara.readTimeout = readTimeout;
        videoThread->setReadTimeout(readTimeout);
    }
}

void VideoWidget::setAspect(double width, double height) {
    if (videoThread) {
        videoThread->setAspect(width, height);
    }
}

qint64 VideoWidget::getPosition() {
    if (videoThread) {
        return videoThread->getPosition();
    } else {
        return 0;
    }
}

void VideoWidget::setPosition(qint64 position) {
    if (videoThread) {
        videoThread->setPosition(position);
    }
}

double VideoWidget::getSpeed() {
    if (videoThread) {
        return videoThread->getSpeed();
    } else {
        return 1;
    }
}

void VideoWidget::setSpeed(double speed) {
    if (videoThread) {
        videoThread->setSpeed(speed);
    }
}

int VideoWidget::getVolume() {
    if (videoThread) {
        return videoThread->getVolume();
    } else {
        return 100;
    }
}

void VideoWidget::setVolume(int volume) {
    //同时限定音量范围
    if (videoThread && volume >= 0 && volume <= 100) {
        widgetPara_.soundValue = volume;
        videoThread->setVolume(volume);
    }
}

bool VideoWidget::getMuted() {
    if (videoThread) {
        return videoThread->getMuted();
    } else {
        return false;
    }
}

void VideoWidget::setMuted(bool muted) {
    if (videoThread) {
        widgetPara_.soundMuted = muted;
        videoThread->setMuted(muted);
    }
}

void VideoWidget::setPlayStep(bool playStep) {
    if (videoThread) {
        videoPara.playStep = playStep;
        videoThread->setPlayStep(playStep);
    }
}

void VideoWidget::resize2() {
    //如果有旋转角度则宽高对调
    if (rotate_ == 90) {
        this->setAspect(height(), width());
    } else {
        this->setAspect(width(), height());
    }
}

void VideoWidget::started() {
    isRunning_ = true;
    this->update();
}

void VideoWidget::finished() {
    isRunning_ = false;
    this->update();
}

void VideoWidget::receivePlayStart(int time) {
    //vlc内核句柄模式下在打开后鼠标打圈圈需要复位下
    if (widgetPara_.videoMode == VideoMode_Hwnd) {
        VideoTask::Instance().append("resetCursor", "");
    }

    //句柄模式下句柄控件要禁用绘制防止闪烁(尤其是海康大华厂家sdk内核)
    bool isQMedia = (videoPara.videoCore == VideoCore_QMedia);
    bool isMdk = (videoPara.videoCore == VideoCore_Mdk);
    bool isQtav = (videoPara.videoCore == VideoCore_Qtav);
    if (widgetPara_.videoMode == VideoMode_Hwnd && !isQMedia && !isMdk && !isQtav) {
        hwndWidget_->setUpdatesEnabled(false);
    }

    //重置码率
    kbps_ = 0;
    //执行父类处理
    AbstractVideoWidget::receivePlayStart(time);

    //设置默认声音大小和静音状态
    this->setVolume(widgetPara_.soundValue);
    this->setMuted(widgetPara_.soundMuted);
    //如果逐帧播放则先暂停并切换一次
    if (videoPara.playStep) {
        this->pause();
        this->step();
    }
}

void VideoWidget::receivePlayFinsh() {
    //mpv内核句柄模式下在关闭后鼠标打圈圈需要复位下
    if (widgetPara_.videoMode == VideoMode_Hwnd) {
        VideoTask::Instance().append("resetCursor", "");
    }

    //句柄模式下句柄控件要禁用绘制防止闪烁(尤其是海康大华厂家sdk内核)
    bool isQMedia = (videoPara.videoCore == VideoCore_QMedia);
    bool isMdk = (videoPara.videoCore == VideoCore_Mdk);
    bool isQtav = (videoPara.videoCore == VideoCore_Qtav);
    if (widgetPara_.videoMode == VideoMode_Hwnd && !isQMedia && !isMdk && !isQtav) {
        hwndWidget_->setUpdatesEnabled(true);
    }

    //执行父类处理
    AbstractVideoWidget::receivePlayFinish();
}

bool VideoWidget::checkReceive(bool clear) {
    //可能在结束前的瞬间也发送过来了但是这边还没来得及绘制那边又释放了帧
    //所以要加个线程是否在运行的判断不然停止播放后可能会崩溃
    if (!isRunning_) {
        //线程停止后设置一张空白图片
        if (clear) {
            AbstractVideoWidget::clear();
        }
        return false;
    }

    return true;
}

void VideoWidget::receiveImage(const QImage &image, int time) {
    if (this->checkReceive(true)) {
        AbstractVideoWidget::receiveImage(image, time);
    }
}

void VideoWidget::receiveFrame(int width, int height, quint8 *dataRGB, int type) {
    if (this->checkReceive()) {
        AbstractVideoWidget::receiveFrame(width, height, dataRGB, type);
    }
}

void VideoWidget::receiveFrame(int width, int height, quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 linesizeY,
                               quint32 linesizeU, quint32 linesizeV) {
    if (this->checkReceive()) {
        AbstractVideoWidget::receiveFrame(width, height, dataY, dataU, dataV, linesizeY, linesizeU, linesizeV);
    }
}

void
VideoWidget::receiveFrame(int width, int height, quint8 *dataY, quint8 *dataUV, quint32 linesizeY, quint32 linesizeUV) {
    if (this->checkReceive()) {
        AbstractVideoWidget::receiveFrame(width, height, dataY, dataUV, linesizeY, linesizeUV);
    }
}

void VideoWidget::receivePoint(int type, const QPoint &point) {
    if (!videoThread) {
        return;
    }
    //处于电子放大阶段才处理(悬浮条按钮切换)
    if (bannerWidget_->getIsCrop()) {
        //限定只有一个电子放大滤镜
        for (auto &graph: listGraph_) {
            if (graph.name == "crop") {
                return;
            }
        }
        //第一步: 鼠标按下开始记住坐标
        //第二步: 鼠标移动绘制选取区域
        //第三步: 鼠标松开发送裁剪滤镜
        if (type == QEvent::MouseButtonPress) {
            lastPoint = point;
        } else if (type == QEvent::MouseMove) {
            GraphInfo graph;
            graph.name = "croptemp";
            graph.rect = WidgetHelper::getRect(lastPoint, point);
            graph.borderWidth = WidgetHelper::getBorderWidth(this);
            this->setGraph(graph);
        } else if (type == QEvent::MouseButtonRelease) {
            GraphInfo graph;
            graph.name = "crop";
            graph.rect = WidgetHelper::getRect(lastPoint, point);
            this->removeGraph("croptemp");
            this->setGraph(graph);
        }
    }
}

void VideoWidget::btnClicked(const QString &btnName) {
    QString flag = widgetPara_.videoFlag;
    QString name = STRDATETIMEMS;
    if (!flag.isEmpty()) {
        name = QString("%1_%2").arg(flag).arg(name);
    }

    if (btnName.endsWith("btnRecord")) {
        QString fileName = QString("%1/%2.mp4").arg(recordPath).arg(name);
        this->recordStart(fileName);
    } else if (btnName.endsWith("btnStop")) {
        this->recordStop();
    } else if (btnName.endsWith("btnSound")) {
        this->setMuted(true);
    } else if (btnName.endsWith("btnMuted")) {
        this->setMuted(false);
    } else if (btnName.endsWith("btnSnap")) {
        QString snapName = QString("%1/%2.jpg").arg(snapPath).arg(name);
        this->snap(snapName, false);
    } else if (btnName.endsWith("btnCrop")) {
        if (videoThread) {
            if (videoPara.videoCore == VideoCore_FFmpeg) {
                QMetaObject::invokeMethod(videoThread, "setCrop", Q_ARG(bool, true));
            }
        }
    } else if (btnName.endsWith("btnReset")) {
        if (videoThread) {
            this->removeGraph("crop");
            if (videoPara.videoCore == VideoCore_FFmpeg) {
                this->removeGraph("croptemp");
                QMetaObject::invokeMethod(videoThread, "setCrop", Q_ARG(bool, false));
            }
        }
    } else if (btnName.endsWith("btnAlarm")) {

    } else if (btnName.endsWith("btnClose")) {
        this->stop();
    }
}

void VideoWidget::osdChanged() {
    if (videoThread && widgetPara_.osdDrawMode == DrawMode_Source) {
        videoThread->setOsdInfo(listOsd_);
    }
}

void VideoWidget::graphChanged() {
    if (videoThread && widgetPara_.graphDrawMode == DrawMode_Source) {
        videoThread->setGraphInfo(listGraph_);
    }
}

void VideoWidget::connectThreadSignal() {
    if (!videoThread) {
        return;
    }
    //后面带个参数指定信号唯一(如果多次连接信号会自动去重)
    connect(videoThread, &VideoThread::started, this, &VideoWidget::started);
    connect(videoThread, &VideoThread::finished, this, &VideoWidget::finished);
    connect(videoThread, &VideoThread::receivePlayStart, this, &VideoWidget::receivePlayStart);
    connect(videoThread, &VideoThread::receivePlayFinish, this, &VideoWidget::receivePlayFinsh);

    connect(videoThread, &VideoThread::receiveImage, this, &VideoWidget::receiveImage);
    connect(videoThread, &VideoThread::snapImage, this, &VideoWidget::snapImage);
    connect(videoThread, QOverload<int, int, quint8 *, int>::of(&VideoThread::receiveFrame), this,
            QOverload<int, int, quint8 *, int>::of(&VideoWidget::receiveFrame));
    connect(videoThread, QOverload<int, int, quint8 *, quint8 *, quint8 *, quint32, quint32, quint32>::of(
                    &VideoThread::receiveFrame), this,
            QOverload<int, int, quint8 *, quint8 *, quint8 *, quint32, quint32, quint32>::of(
                    &VideoWidget::receiveFrame));
    connect(videoThread, QOverload<int, int, quint8 *, quint8 *, quint32, quint32>::of(
                    &VideoThread::receiveFrame), this,
            QOverload<int, int, quint8 *, quint8 *, quint32, quint32>::of(&VideoWidget::receiveFrame));

    connect(videoThread, &VideoThread::receiveLevel, this, &VideoWidget::sigReceiveLevel);
    connect(videoThread, &VideoThread::receiveKbps, this, &VideoWidget::sigReceiveKbps);
    connect(videoThread, &VideoThread::receivePlayStart, this, &VideoWidget::sigReceivePlayStart);
    connect(videoThread, &VideoThread::receivePlayFinish, this, &VideoWidget::sigReceivePlayFinish);

    connect(videoThread, &VideoThread::receivePlayFinish, bannerWidget_, &BannerWidget::receivePlayFinish);
    connect(videoThread, &VideoThread::receiveMuted, bannerWidget_, &BannerWidget::receiveMuted);
    connect(videoThread, &VideoThread::recorderStateChanged, bannerWidget_, &BannerWidget::recorderStateChanged);
    connect(videoThread, &VideoThread::receiveSizeChanged, this, &VideoWidget::receiveSizeChanged);

    //根据默认音量大小和静音状态触发下信号
    if (videoPara.videoCore == VideoCore_FFmpeg) {
        QMetaObject::invokeMethod(videoThread, "receiveVolume", Q_ARG(int, widgetPara_.soundValue));
        QMetaObject::invokeMethod(videoThread, "receiveMuted", Q_ARG(bool, widgetPara_.soundMuted));
    }
}

void VideoWidget::disconnectThreadSignal() {
    if (!videoThread) {
        return;
    }

    disconnect(videoThread, &VideoThread::started, this, &VideoWidget::started);
    disconnect(videoThread, &VideoThread::finished, this, &VideoWidget::finished);
    disconnect(videoThread, &VideoThread::receivePlayStart, this, &VideoWidget::receivePlayStart);
    disconnect(videoThread, &VideoThread::receivePlayFinish, this, &VideoWidget::receivePlayFinish);

    disconnect(videoThread, &VideoThread::receiveImage, this, &VideoWidget::receiveImage);
    disconnect(videoThread, &VideoThread::snapImage, this, &VideoWidget::snapImage);
    disconnect(videoThread, QOverload<int, int, quint8 *, int>::of(&VideoThread::receiveFrame), this,
               QOverload<int, int, quint8 *, int>::of(&VideoWidget::receiveFrame));

    disconnect(videoThread, QOverload<int, int, quint8 *, quint8 *, quint8 *, quint32, quint32, quint32>::of(
                       &VideoThread::receiveFrame), this,
               QOverload<int, int, quint8 *, quint8 *, quint8 *, quint32, quint32, quint32>::of(
                       &VideoWidget::receiveFrame));

    disconnect(videoThread, QOverload<int, int, quint8 *, quint8 *, quint32, quint32>::of(&VideoThread::receiveFrame),
               this, QOverload<int, int, quint8 *, quint8 *, quint32, quint32>::of(&VideoWidget::receiveFrame));

    disconnect(videoThread, &VideoThread::receiveLevel, this, &VideoWidget::sigReceiveLevel);
    disconnect(videoThread, &VideoThread::receiveKbps, this, &VideoWidget::sigReceiveKbps);
    disconnect(videoThread, &VideoThread::receivePlayStart, this, &VideoWidget::sigReceivePlayStart);
    disconnect(videoThread, &VideoThread::receivePlayFinish, this, &VideoWidget::sigReceivePlayFinish);

    disconnect(videoThread, &VideoThread::receivePlayFinish, bannerWidget_, &BannerWidget::receivePlayFinish);
    disconnect(videoThread, &VideoThread::receiveMuted, bannerWidget_, &BannerWidget::receiveMuted);
    disconnect(videoThread, &VideoThread::recorderStateChanged, bannerWidget_,
               &BannerWidget::recorderStateChanged);
    disconnect(videoThread, &VideoThread::receiveSizeChanged, this, &VideoWidget::receiveSizeChanged);
}

bool VideoWidget::init() {
    //媒体地址不能为空
    if (videoPara.mediaUrl.isEmpty()) {
        return false;
    }

    //如果没有解码内核则不用继续
    if (videoPara.videoCore == VideoCore_None) {
        return false;
    }

    //初始化参数
    VideoHelper::initPara(widgetPara_, videoPara, encodePara_);
    //线程正在运行不用继续
    if (isRunning_) {
        return false;
    }

    //句柄模式则句柄控件在前否则遮罩控件在前
    if (widgetPara_.videoMode == VideoMode_Hwnd) {
        coverWidget_->stackUnder(hwndWidget_);
    } else {
        hwndWidget_->stackUnder(coverWidget_);
    }

    //已经存在同名的线程则取同名线程
    VideoThread *thread = VideoThread::getVideoThread(widgetPara_, videoPara);
    if (thread) {
        isShared_ = true;
        videoThread = thread;
        //默认音量大小和静音状态取共享线程的
        widgetPara_.soundValue = thread->getVolume();
        widgetPara_.soundMuted = thread->getMuted();
        //硬件加速也要取共享线程的
        hardware_ = thread->getHardware();
        videoPara.hardware = thread->getHardware();
    } else {
        //创建新的采集线程
        videoThread = VideoHelper::newVideoThread(hwndWidget_, videoPara.videoCore, widgetPara_.videoMode);
        //设置视频通道唯一标识
        videoThread->setFlag(widgetPara_.videoFlag);
        //加入到采集线程队列
        if (widgetPara_.sharedData) {
            VideoThread::videoThreads << videoThread;
        }

        //设置对应参数
        videoThread->setVideoMode(widgetPara_.videoMode);
        videoThread->setProperty("scaleMode", widgetPara_.scaleMode);
        videoThread->setProperty("sharedData", widgetPara_.sharedData);
        VideoHelper::initVideoThread(videoThread, videoPara, encodePara_);
    }

    //绑定信号槽
    connectThreadSignal();
    return true;
}

bool VideoWidget::open(const QString &mediaUrl) {
    //线程正常说明还在运行需要先停止
    if (videoThread) {
        this->stop();
        qApp->processEvents();
    }

    //重新初始化和播放
    videoPara.mediaUrl = mediaUrl;
    if (this->init()) {
        this->play();
        return true;
    } else {
        return false;
    }
}

void VideoWidget::play() {
    //如果是图片则只显示图片就行
    image_ = QImage(videoPara.mediaUrl);
    if (!image_.isNull()) {
        videoThread->setVideoSize(QString("%1x%2").arg(image_.width()).arg(image_.height()));
        this->setImage(image_);
        return;
    }

    //采用已经存在的采集线程/如果处于解码正常阶段则还要发几个信号通知
    if (videoThread->getIsOk()) {
        QMetaObject::invokeMethod(this, "receivePlayStart", Qt::DirectConnection, Q_ARG(int, 0));
        QMetaObject::invokeMethod(videoThread, "receiveSizeChanged", Qt::DirectConnection);
        QMetaObject::invokeMethod(videoThread, "receiveDuration", Qt::DirectConnection,
                                  Q_ARG(qint64, videoThread->getDuration()));
    }

    //已经在运行阶段还要发送已经开始的信号
    if (videoThread->isRunning()) {
        isRunning_ = true;
        QMetaObject::invokeMethod(this, "sig_receivePlayStart", Qt::DirectConnection, Q_ARG(int, 0));
    }

    //启动播放线程
    videoThread->play();
    this->update();
    //初始化标签信息和图形信息
    this->osdChanged();
    this->graphChanged();
}

void VideoWidget::stop() {
    //立即隐藏悬浮条
    bannerWidget_->setVisible(false);
    //关闭的时候将遮罩控件移到最前
    hwndWidget_->stackUnder(coverWidget_);

    //处于运行状态才可以停止
    if (videoThread && videoThread->isRunning()) {
        //先判断当前线程的引用计数是0才需要真正停止
        if (videoThread->refCount > 0) {
            isRunning_ = false;
            //减少引用计数
            videoThread->refCount--;
            //执行停止信号
            QMetaObject::invokeMethod(this, "receivePlayFinish", Qt::DirectConnection);
            QMetaObject::invokeMethod(this, "sigReceivePlayFinish", Qt::DirectConnection);
            //取消信号关联
            disconnectThreadSignal();
        } else {
            //停止播放
            videoThread->stop();
            //取消信号关联
            disconnectThreadSignal();
            //从队列中移除
            VideoThread::videoThreads.removeOne(videoThread);
            //释放线程
            videoThread->debug("删除线程", "");
            videoThread->deleteLater();
        }
    }

    //复位标志位并将线程置空(没有这个的话可能会出现野指针的情况)
    isRunning_ = false;
    isShared_ = false;
    videoThread = nullptr;
    videoPara.reset();
    AbstractVideoWidget::clear();
}

void VideoWidget::pause() {
    if (videoThread) {
        videoThread->pause();
    }
}

void VideoWidget::next() {
    if (videoThread) {
        videoThread->next();
    }
}

void VideoWidget::step(bool backward) {
    if (videoThread && !onlyAudio_) {
        videoThread->step(backward);
    }
}

void VideoWidget::snap(const QString &snapName, bool preview) {
    if (videoThread && !onlyAudio_) {
        this->preview_ = preview;
        videoThread->snap(snapName);
    }
}

void VideoWidget::recordStart(const QString &fileName) {
    if (videoThread && videoThread->getIsOk()) {
        videoThread->recordStart(fileName);
    }
}

void VideoWidget::recordPause() {
    if (videoThread && videoThread->getIsOk()) {
        videoThread->recordPause();
    }
}

void VideoWidget::recordStop() {
    if (videoThread && videoThread->getIsOk()) {
        videoThread->recordStop();
    }
}
