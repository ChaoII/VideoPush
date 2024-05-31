#include "frmsimple.h"
#include "ui_frmsimple.h"
#include "qthelper.h"

#ifdef netpush
#include "urlutil.h"
#include "urlhelper.h"
#include "videopushurl.h"
#include "netpushserver.h"
#endif

frmSimple::frmSimple(QWidget *parent) : QWidget(parent), ui(new Ui::frmSimple)
{
    ui->setupUi(this);
    this->initForm();
    this->initConfig();
}

frmSimple::~frmSimple()
{
    delete ui;
}

void frmSimple::initForm()
{
    pushServer = NULL;
}

void frmSimple::initConfig()
{
#ifdef netpush
    ui->cboxMediaUrl->addItems(UrlUtil::getUrls(UrlUtil::Simple));
#endif
    ui->cboxMediaUrl->lineEdit()->setText(AppConfig::SimpleMediaUrl);
    connect(ui->cboxMediaUrl->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    ui->txtPushUrl->setText(AppConfig::SimplePushUrl);
    connect(ui->txtPushUrl, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));
}

void frmSimple::saveConfig()
{
    AppConfig::SimpleMediaUrl = ui->cboxMediaUrl->lineEdit()->text().trimmed();
    AppConfig::SimplePushUrl = ui->txtPushUrl->text().trimmed();
    AppConfig::writeConfig();
}

void frmSimple::on_btnSelect_clicked()
{
    QString filter = "视频文件(*.mp4 *.rmvb *.avi *.asf *.mov *.wmv *.mkv);;音频文件(*.mp3 *.wav *.wma *.aac);;所有文件(*.*)";
    QString fileName = QFileDialog::getOpenFileName(this, "", "", filter);
    if (!fileName.isEmpty()) {
        ui->cboxMediaUrl->lineEdit()->setText(fileName);
    }
}

void frmSimple::on_btnStart_clicked()
{
#if 0
    FFmpegSaveSimple *f = new FFmpegSaveSimple(this);
    f->setUrl("f:/mp4/push/1.mp4", "rtmp://192.168.0.110:1936/stream/s1");
    f->start();
    return;
#endif
#ifdef netpush
    if (!pushServer) {
        pushServer = new NetPushServer;
    }

    if (ui->btnStart->text() == "启动服务") {
        QString flag;
        QString mediaUrl = ui->cboxMediaUrl->lineEdit()->text().trimmed();
        QString pushUrl = ui->txtPushUrl->text().trimmed();
        pushServer->setPushUrl(pushUrl);
        pushServer->addUrl(mediaUrl, flag);
        pushServer->start();
        ui->btnStart->setText("停止服务");

        //取出拉流地址
        QString text = pushServer->getPushUrl(mediaUrl);
        //自动拷贝到剪切板方便直接粘贴测试使用
        if (AppConfig::AutoCopy) {
            qApp->clipboard()->setText(text);
        }

        //自动填充地址(推流后即可用rtmp/rtsp拉流也可用http拉流)
        QString host = UrlHelper::getUrlHost(text);
        QString path = VideoPushUrl::getPushPath(text);
        ui->txtRtspUrl->setText(QString("rtsp://%1:8554%2").arg(host).arg(path));
        ui->txtRtmpUrl->setText(QString("rtmp://%1:1935%2").arg(host).arg(path));
        ui->txtHlsUrl->setText(QString("http://%1:8888%2").arg(host).arg(path));
        ui->txtM3u8Url->setText(QString("http://%1:8888%2/index.m3u8").arg(host).arg(path));
        ui->txtWebRtcUrl->setText(QString("http://%1:8889%2").arg(host).arg(path));
    } else {
        pushServer->stop();
        ui->btnStart->setText("启动服务");
        ui->txtRtspUrl->clear();
        ui->txtRtmpUrl->clear();
        ui->txtHlsUrl->clear();
        ui->txtM3u8Url->clear();
        ui->txtWebRtcUrl->clear();
    }
#endif
}
