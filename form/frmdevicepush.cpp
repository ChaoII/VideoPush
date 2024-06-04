#include "frmdevicepush.h"
#include "ui_frmdevicepush.h"
#include "qthelper.h"
#include "widgethelper.h"
#include "videohelper.h"
#include "deviceinfohelper.h"
#include "ffmpegthread.h"
#include "netpushclient.h"

FormDevicePush::FormDevicePush(QWidget *parent) : QWidget(parent), ui(new Ui::FormDevicePush) {
    ui->setupUi(this);
    this->initForm();
    this->initDevice();
    this->initConfig();
}

FormDevicePush::~FormDevicePush() {
    AppConfig::DevicePushStart1 = (ui->btnStart1->text() == "停止推流");
    AppConfig::DevicePushStart2 = (ui->btnStart2->text() == "停止推流");
    AppConfig::writeConfig();
    delete ui;
}

void FormDevicePush::initForm() {
    BannerInfo bannerInfo = ui->videoWidget->getBannerInfo();
    bannerInfo.enableAll();
    ui->videoWidget->setBannerInfo(bannerInfo);

    WidgetPara widgetPara = ui->videoWidget->getWidgetPara();
    widgetPara.bannerEnable = true;
    ui->videoWidget->setWidgetPara(widgetPara);

    VideoPara videoPara = ui->videoWidget->getVideoPara();
    videoPara.videoCore = VideoCore_FFmpeg;
    videoPara.audioLevel = true;
    ui->videoWidget->setVideoPara(videoPara);
    connect(ui->videoWidget, &VideoWidget::sigReceiveLevel, ui->audioLevel, &AudioLevel::setLevel);

    //实例化音频采集线程
    ffmpegThread = new FFmpegThread;
    ffmpegThread->setAudioLevel(true);
    connect(ffmpegThread, &FFmpegThread::receivePlayStart, this, &FormDevicePush::receivePlayStart);
    connect(ffmpegThread, &FFmpegThread::receivePlayFinish, this, &FormDevicePush::receivePlayFinish);
    connect(ffmpegThread, &FFmpegThread::receivePlayFinish, ui->audioLevel, &AudioLevel::clear);
    connect(ffmpegThread, &FFmpegThread::receiveLevel, ui->audioLevel, &AudioLevel::setLevel);

    connect(ui->videoWidget, &VideoWidget::sigReceivePlayStart, this, &FormDevicePush::receivePlayStart);
    connect(ui->videoWidget, &VideoWidget::sigReceivePlayFinish, this, &FormDevicePush::receivePlayFinish);

    connect(ui->cboxVideoDevice, &QComboBox::activated, this, &FormDevicePush::initUrl);
    connect(ui->cboxAudioDevice, &QComboBox::activated, this, &FormDevicePush::initUrl);

    connect(ui->ckMuted, &QCheckBox::stateChanged, this, &FormDevicePush::initPara);
    connect(ui->ckOsd, &QCheckBox::stateChanged, this, &FormDevicePush::initPara);
}

void FormDevicePush::initConfig() {
    //试用版永远有水印
#ifdef betaversion
    AppConfig::DevicePushOsd = true;
    ui->ckOsd->setEnabled(false);
#endif

    int index = ui->cboxVideoDevice->findText(AppConfig::DeviceVideo);
    index = (index < 0 ? 0 : index);
    ui->cboxVideoDevice->setCurrentIndex(index);
    connect(ui->cboxVideoDevice, &QComboBox::currentIndexChanged, this, &FormDevicePush::saveConfig);

    index = ui->cboxAudioDevice->findText(AppConfig::DeviceAudio);
    index = (index < 0 ? 0 : index);
    ui->cboxAudioDevice->setCurrentIndex(index);
    connect(ui->cboxAudioDevice, &QComboBox::currentIndexChanged, this, &FormDevicePush::saveConfig);

    ui->txtMediaUrl1->setText(AppConfig::DeviceMediaUrl1);
    connect(ui->txtMediaUrl1, &QLineEdit::textChanged, this, &FormDevicePush::saveConfig);

    ui->txtMediaUrl2->setText(AppConfig::DeviceMediaUrl2);
    connect(ui->txtMediaUrl2, &QLineEdit::textChanged, this, &FormDevicePush::saveConfig);

    ui->txtPushUrl1->setText(AppConfig::DevicePushUrl1);
    connect(ui->txtPushUrl1, &QLineEdit::textChanged, this, &FormDevicePush::saveConfig);

    ui->txtPushUrl2->setText(AppConfig::DevicePushUrl2);
    connect(ui->txtPushUrl2, &QLineEdit::textChanged, this, &FormDevicePush::saveConfig);

    ui->ckOsd->setChecked(AppConfig::DevicePushOsd);
    connect(ui->ckOsd, &QCheckBox::checkStateChanged, this, &FormDevicePush::saveConfig);

    ui->ckMuted->setChecked(AppConfig::DevicePushMuted);
    connect(ui->ckMuted, &QCheckBox::checkStateChanged, this, &FormDevicePush::saveConfig);

    //自动启动推流
    if (AppConfig::DevicePushStart1) {
        on_btnStart1_clicked();
    }
    if (AppConfig::DevicePushStart2) {
        on_btnStart2_clicked();
    }
}

void FormDevicePush::saveConfig() {
    AppConfig::DeviceVideo = ui->cboxVideoDevice->currentText();
    AppConfig::DeviceAudio = ui->cboxAudioDevice->currentText();
    AppConfig::DeviceMediaUrl1 = ui->txtMediaUrl1->text().trimmed();
    AppConfig::DeviceMediaUrl2 = ui->txtMediaUrl2->text().trimmed();
    AppConfig::DevicePushUrl1 = ui->txtPushUrl1->text().trimmed();
    AppConfig::DevicePushUrl2 = ui->txtPushUrl2->text().trimmed();
    AppConfig::DevicePushOsd = ui->ckOsd->isChecked();
    AppConfig::DevicePushMuted = ui->ckMuted->isChecked();
    AppConfig::writeConfig();
}

void FormDevicePush::initDevice() {
    QStringList audioDevices, videoDevices;
    DeviceInfoHelper::getInputDevices(audioDevices, videoDevices);

    ui->cboxAudioDevice->clear();
    ui->cboxVideoDevice->clear();
    ui->cboxAudioDevice->addItems(audioDevices);
    ui->cboxVideoDevice->addItems(videoDevices);
}

void FormDevicePush::initUrl() {
    //如果音频设备切换的时候视频地址带了音频设备则不处理
    if (sender() == ui->cboxAudioDevice && AppConfig::DeviceMediaUrl1.contains(":audio")) {
        //return;
    }
    QString videoDevice = ui->cboxVideoDevice->currentText();
    QString audioDevice = ui->cboxAudioDevice->currentText();

    if (!videoDevice.isEmpty()) {
        //取出原来的地址后面参数部分
        QString mediaUrl = ui->txtMediaUrl1->text().trimmed();
        QStringList list = mediaUrl.split("|");
        QString url;
        if (videoDevice.startsWith("DISPLAY")) {
            url = DeviceInfoHelper::getScreenUrl(videoDevice);
        } else {
            url = "video=" + videoDevice;
            //存在音频设备则还要替换对应的音频设备
            if (mediaUrl.contains(":audio=")) {
                url = url + ":audio=" + audioDevice;
            }
            list[0] = url;
            url = list.join("|");
        }
        ui->txtMediaUrl1->setText(url);
    }
    if (!audioDevice.isEmpty()) {
        QString url = "audio=" + audioDevice;
        ui->txtMediaUrl2->setText(url);
    }
}

void FormDevicePush::initPara() {
    //本地麦克风采集声音后会发送给默认的声卡播放/可能导致回音/可以取消
    bool muted = ui->ckMuted->isChecked();
    if (ffmpegThread->isRunning()) {
        ffmpegThread->setMuted(muted);
    }
    if (ui->videoWidget->getIsRunning()) {
        //如果是本地桌面采集带音频则需要静音/否则声音一直重复播放导致滴滴滴
        ui->videoWidget->setMuted(true);
        //显示个日期时间标签水印/方便对比延迟
        if (ui->ckOsd->isChecked()) {
            int height = ui->videoWidget->getVideoHeight();
            QList<OsdInfo> osds = WidgetHelper::getTestOsd(height);
            for (auto &osd: osds) {
                ui->videoWidget->setOsd(osd);
            }
        } else {
            ui->videoWidget->clearOsd();
        }
    }
}

void FormDevicePush::receivePlayStart(int time) {
    this->initPara();
    if (sender() == ffmpegThread) {
        ffmpegThread->recordStart(AppConfig::DevicePushUrl2);
        ui->btnStart2->setText("停止推流");
    } else {
        VideoThread *thread = ui->videoWidget->getVideoThread();
        thread->recordStart(AppConfig::DevicePushUrl1);
        ui->btnStart1->setText("停止推流");
    }
}

void FormDevicePush::receivePlayFinish() {
    if (sender() == ffmpegThread) {
        ui->btnStart2->setText("启动推流");
    } else {
        ui->btnStart1->setText("启动推流");
    }
}

void FormDevicePush::on_btnStart1_clicked() {
    if (ui->btnStart1->text() == "启动推流") {
        ui->videoWidget->open(AppConfig::DeviceMediaUrl1);
    } else {
        ui->videoWidget->stop();
    }
}

void FormDevicePush::on_btnStart2_clicked() {
    if (ui->btnStart2->text() == "启动推流") {
        ffmpegThread->setMediaUrl(AppConfig::DeviceMediaUrl2);
        ffmpegThread->play();
    } else {
        ffmpegThread->stop();
    }
}
