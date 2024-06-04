#include "frmspeedpush.h"
#include "ui_frmspeedpush.h"
#include "qthelper.h"
#include "videoutil.h"

frmSpeedPush::frmSpeedPush(QWidget *parent) : QWidget(parent), ui(new Ui::frmSpeedPush) {
    ui->setupUi(this);
    this->initForm();
    this->initConfig();
}

frmSpeedPush::~frmSpeedPush() {
    AppConfig::SpeedPushStart = (ui->btnStart->text() == "停止推流");
    AppConfig::writeConfig();
    delete ui;
}

void frmSpeedPush::initForm() {
    VideoPara videoPara = ui->videoWidget->getVideoPara();
    videoPara.videoCore = VideoCore_FFmpeg;
    ui->videoWidget->setVideoPara(videoPara);

    VideoPara para = ui->videoWidget->getVideoPara();
    para.playRepeat = true;
    ui->videoWidget->setVideoPara(para);

    connect(ui->videoWidget, &VideoWidget::sigReceivePlayStart, this, &frmSpeedPush::receivePlayStart);
    connect(ui->videoWidget, &VideoWidget::sigReceivePlayFinish, this, &frmSpeedPush::receivePlayFinish);
}

void frmSpeedPush::initConfig() {
    VideoUtil::loadMediaUrl(ui->cboxMediaUrl, AppConfig::SpeedMediaUrl, 0x40);
    connect(ui->cboxMediaUrl->lineEdit(), &QLineEdit::textChanged, this, &frmSpeedPush::saveConfig);

    ui->txtPushUrl->setText(AppConfig::SpeedPushUrl);
    connect(ui->txtPushUrl, &QLineEdit::textChanged, this, &frmSpeedPush::saveConfig);

    VideoUtil::loadSpeed(ui->cboxSpeed);
    ui->cboxSpeed->setCurrentIndex(ui->cboxSpeed->findData(AppConfig::SpeedPushValue));
    connect(ui->cboxSpeed, &QComboBox::currentIndexChanged, this, &frmSpeedPush::saveConfig);

    ui->ckMuted->setChecked(AppConfig::SpeedPushMuted);
    connect(ui->ckMuted, &QCheckBox::checkStateChanged, this, &frmSpeedPush::saveConfig);

    if (AppConfig::SpeedPushStart) {
        on_btnStart_clicked();
    }
}

void frmSpeedPush::saveConfig() {
    AppConfig::SpeedMediaUrl = ui->cboxMediaUrl->currentText().trimmed();
    AppConfig::SpeedPushUrl = ui->txtPushUrl->text().trimmed();
    AppConfig::SpeedPushValue = ui->cboxSpeed->itemData(ui->cboxSpeed->currentIndex()).toFloat();
    AppConfig::SpeedPushMuted = ui->ckMuted->isChecked();
    AppConfig::writeConfig();
}

void frmSpeedPush::receivePlayStart(int time) {
    Q_UNUSED(time)
    VideoThread *thread = ui->videoWidget->getVideoThread();
    thread->setMuted(AppConfig::SpeedPushMuted);
    thread->setSpeed(AppConfig::SpeedPushValue);
    thread->setEncodeSpeed(AppConfig::SpeedPushValue);
    thread->recordStart(AppConfig::SpeedPushUrl);
    connect(thread, &VideoThread::receivePosition, this, &frmSpeedPush::receivePosition);

    ui->sliderPosition->setRange(0, static_cast<int>(thread->getDuration()));
    ui->btnStart->setText("停止推流");
    ui->cboxSpeed->setEnabled(false);
}

void frmSpeedPush::receivePlayFinish() {
    ui->sliderPosition->setRange(0, 0);
    ui->btnStart->setText("启动推流");
    ui->cboxSpeed->setEnabled(true);
}

void frmSpeedPush::receivePosition(qint64 position) {
    ui->sliderPosition->setValue(static_cast<int>(position));
}

void frmSpeedPush::on_btnStart_clicked() {
    if (ui->btnStart->text() == "启动推流") {
        ui->videoWidget->open(AppConfig::SpeedMediaUrl);
    } else {
        ui->videoWidget->stop();
    }
}

void frmSpeedPush::on_sliderPosition_clicked() {
    int value = ui->sliderPosition->value();
    on_sliderPosition_sliderMoved(value);
}

void frmSpeedPush::on_sliderPosition_sliderMoved(int value) {
    ui->videoWidget->setPosition(value);
}

void frmSpeedPush::on_ckMuted_stateChanged(int arg1) {
    Q_UNUSED(arg1)
    ui->videoWidget->setMuted(ui->ckMuted->isChecked());
}
