#include "frmimagepush.h"
#include "ui_frmimagepush.h"
#include "qthelper.h"
#include "imagepushserver.h"
#include "qthelper.h"

frmImagePush::frmImagePush(QWidget *parent) : QWidget(parent), ui(new Ui::frmImagePush)
{
    ui->setupUi(this);
    this->initForm();
    this->initConfig();
}

frmImagePush::~frmImagePush()
{
    if (ui->btnStart->text() == "停止服务") {
        pushServer->stop();
    }

    delete ui;
}

void frmImagePush::initForm()
{
    pushServer = new ImagePushServer(this);
    connect(pushServer, SIGNAL(receiveCount(int)), this, SLOT(receiveCount(int)));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(test()));
    timer->start(1000 / 20);
}

void frmImagePush::initConfig()
{
    QtHelper::initLocalIPs(ui->cboxListenIP, AppConfig::ImageListenIP);
    connect(ui->cboxListenIP->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    ui->txtListenPort->setText(QString::number(AppConfig::ImageListenPort));
    connect(ui->txtListenPort, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    if (AppConfig::ImagePushStart) {
        on_btnStart_clicked();
    }
}

void frmImagePush::saveConfig()
{
    AppConfig::ImageListenIP = ui->cboxListenIP->lineEdit()->text().trimmed();
    AppConfig::ImageListenPort = ui->txtListenPort->text().trimmed().toInt();
    AppConfig::writeConfig();
}

void frmImagePush::test()
{
    QImage image(400, 300, QImage::Format_RGB32);
    image.fill(Qt::transparent);
    QPainter p;
    p.begin(&image);
    QString str = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss zzz");
    p.setPen(Qt::red);
    QFontMetrics metrics(p.font());
    int textWidth = metrics.horizontalAdvance(str);
    int textHeight = metrics.height();
    p.drawText(10, 10, textWidth, textHeight, 1, str);
    p.end();

    pushServer->sendImage(image);
}

void frmImagePush::receiveCount(int count)
{
    ui->lcdNumber->display(count);
}

void frmImagePush::on_btnStart_clicked()
{
    if (ui->btnStart->text() == "启动服务") {
        if (pushServer->start(AppConfig::ImageListenIP, AppConfig::ImageListenPort)) {
            ui->btnStart->setText("停止服务");
            ui->widgetPara->setEnabled(false);
        }
    } else {
        pushServer->stop();
        ui->btnStart->setText("启动服务");
        ui->widgetPara->setEnabled(true);
    }

    AppConfig::ImagePushStart = (ui->btnStart->text() == "停止服务");
    AppConfig::writeConfig();
}

void frmImagePush::on_btnRemove_clicked()
{

}

void frmImagePush::on_btnClear_clicked()
{

}

void frmImagePush::on_btnSendImage_clicked()
{
    QString filter = "图片文件(*.jpg *.png *,jpeg *.bmp)";
    QString fileName = QFileDialog::getOpenFileName(this, "", "", filter);
    if (!fileName.isEmpty()) {
        pushServer->sendImage(QImage(fileName));
    }
}

void frmImagePush::on_btnOpenCamera_clicked()
{

}
