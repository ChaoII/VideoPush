#include "frmnetadd.h"
#include "ui_frmnetadd.h"
#include "qthelper.h"
#include "ffmpegutil.h"

frmNetAdd::frmNetAdd(QWidget *parent) : QDialog(parent), ui(new Ui::frmNetAdd)
{
    ui->setupUi(this);
    this->initForm();
    QtHelper::setFormInCenter(this);
}

frmNetAdd::~frmNetAdd()
{
    delete ui;
}

void frmNetAdd::initForm()
{
    this->setWindowTitle("添加地址");
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(ui->cboxType, SIGNAL(currentIndexChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->txtRtsp, SIGNAL(textChanged(QString)), this, SLOT(initUrl()));
    connect(ui->cboxTransport, SIGNAL(currentIndexChanged(int)), this, SLOT(initUrl()));

    //ffmpeg5及以上才能正常获取到设备列表
    ui->cboxAudioDevice->addItems(FFmpegUtil::getInputDevices(false));
    ui->cboxVideoDevice->addItems(FFmpegUtil::getInputDevices(true));
    connect(ui->cboxAudioDevice, SIGNAL(currentIndexChanged(int)), this, SLOT(initUrl()));
    connect(ui->cboxVideoDevice, SIGNAL(currentIndexChanged(int)), this, SLOT(initUrl()));
    connect(ui->cboxResolution->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(initUrl()));
    connect(ui->cboxFrameRate->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(initUrl()));
}

void frmNetAdd::initUrl()
{
    int index = ui->cboxType->currentIndex();
    if (index == 0) {
        QString url = ui->txtRtsp->text().trimmed();
        if (url.isEmpty() || !url.startsWith("rtsp")) {
            return;
        }

        QString transport = ui->cboxTransport->currentText();
        if (transport != "none") {
            url = url + "|" + transport;
        }

        ui->cboxUrl->lineEdit()->setText(url);
    } else if (index == 1) {
        QString audioDevice = ui->cboxAudioDevice->currentText();
        QString videoDevice = ui->cboxVideoDevice->currentText();
        if (audioDevice == "none" && videoDevice == "none") {
            return;
        }

        QStringList list;
        if (videoDevice != "none") {
            list << QString("video=%1").arg(videoDevice);
        }
        if (audioDevice != "none") {
            list << QString("audio=%1").arg(audioDevice);
        }

        QString url = list.join(":");
        QString resolution = ui->cboxResolution->currentText().trimmed();
        QString frameRate = ui->cboxFrameRate->currentText().trimmed();

        list.clear();
        list << url;
        if (resolution != "none" || frameRate != "none") {
            list << (resolution == "none" ? "" : resolution);
            list << (frameRate == "none" ? "" : frameRate);
        }

        ui->cboxUrl->lineEdit()->setText(list.join("|"));
    } else if (index == 2) {

    }
}

void frmNetAdd::on_btnOk_clicked()
{
    QString flag = ui->txtFlag->text().trimmed();
    QString url = ui->cboxUrl->lineEdit()->text().trimmed();
    emit addUrl(flag, url, true);
    this->close();
}
