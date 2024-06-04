#include "frmsimple.h"
#include "ui_frmsimple.h"
#include "qthelper.h"

#include "urlutil.h"
#include "urlhelper.h"
#include "videopushurl.h"
#include "netpushserver.h"


frmSimple::frmSimple(QWidget *parent) : QWidget(parent), ui(new Ui::frmSimple) {
    ui->setupUi(this);
    this->initConfig();
}

frmSimple::~frmSimple() {
    delete pushServer_;
    delete ui;
}


void frmSimple::initConfig() {

    ui->cboxMediaUrl->addItems(UrlUtil::getUrls(UrlUtil::Simple));
    ui->cboxMediaUrl->lineEdit()->setText(AppConfig::SimpleMediaUrl);
    connect(ui->cboxMediaUrl->lineEdit(), &QLineEdit::textChanged, this, &frmSimple::saveConfig);

    ui->txtPushUrl->setText(AppConfig::SimplePushUrl);
    connect(ui->txtPushUrl, &QLineEdit::textChanged, this, &frmSimple::saveConfig);
}

void frmSimple::saveConfig() {
    AppConfig::SimpleMediaUrl = ui->cboxMediaUrl->lineEdit()->text().trimmed();
    AppConfig::SimplePushUrl = ui->txtPushUrl->text().trimmed();
    AppConfig::writeConfig();
}

void frmSimple::on_btnSelect_clicked() {
    QString filter = "视频文件(*.mp4 *.rmvb *.avi *.asf *.mov *.wmv *.mkv);;音频文件(*.mp3 *.wav *.wma *.aac);;所有文件(*.*)";
    QString fileName = QFileDialog::getOpenFileName(this, "", "", filter);
    if (!fileName.isEmpty()) {
        ui->cboxMediaUrl->lineEdit()->setText(fileName);
    }
}

void frmSimple::on_btnStart_clicked() {
    if (!pushServer_) {
        pushServer_ = new NetPushServer;
    }
    if (ui->btnStart->text() == "启动服务") {
        QString flag;
        QString mediaUrl = ui->cboxMediaUrl->lineEdit()->text().trimmed();
        QString pushUrl = ui->txtPushUrl->text().trimmed();
        pushServer_->setPushUrl(pushUrl);
        pushServer_->addUrl(mediaUrl, flag);
        pushServer_->start();
        ui->btnStart->setText("停止服务");
        //取出拉流地址
        QString text = pushServer_->getPushUrl(mediaUrl);
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
        pushServer_->stop();
        ui->btnStart->setText("启动服务");
        ui->txtRtspUrl->clear();
        ui->txtRtmpUrl->clear();
        ui->txtHlsUrl->clear();
        ui->txtM3u8Url->clear();
        ui->txtWebRtcUrl->clear();
    }

}
