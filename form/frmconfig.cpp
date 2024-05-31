#include "frmconfig.h"
#include "ui_frmconfig.h"
#include "qthelper.h"

FormConfig::FormConfig(QWidget *parent) : QWidget(parent), ui(new Ui::FormConfig) {
    ui->setupUi(this);
    this->initForm();
    this->initConfig();
}

FormConfig::~FormConfig() {
    delete ui;
}

void FormConfig::initForm() {
    ui->gboxBase->setFixedWidth(500);
}

void FormConfig::initConfig() {
    ui->txtWindowTitle->setText(AppConfig::WindowTitle);
    connect(ui->txtWindowTitle, &QLineEdit::textChanged, this, &FormConfig::saveConfig);

    ui->cboxOpenGLType->setCurrentIndex(AppConfig::OpenGLType);
    connect(ui->cboxOpenGLType, &QComboBox::currentIndexChanged, this, &FormConfig::saveConfig);

    ui->ckCheckRun->setChecked(AppConfig::CheckRun);
    connect(ui->ckCheckRun, &QCheckBox::stateChanged, this, &FormConfig::saveConfig);

    ui->ckAutoRun->setChecked(AppConfig::AutoRun);
    connect(ui->ckAutoRun, &QCheckBox::stateChanged, this, &FormConfig::saveConfig);

    ui->ckUseTray->setChecked(AppConfig::UseTray);
    connect(ui->ckUseTray, &QCheckBox::stateChanged, this, &FormConfig::saveConfig);

    ui->ckAutoCopy->setChecked(AppConfig::AutoCopy);
    connect(ui->ckAutoCopy, &QCheckBox::stateChanged, this, &FormConfig::saveConfig);

    ui->txtPublicUrl->setText(AppConfig::PublicUrl);
    connect(ui->txtPublicUrl, &QLineEdit::textChanged, this, &FormConfig::saveConfig);

    ui->txtHttpUrl->setText(AppConfig::HttpUrl);
    connect(ui->txtHttpUrl, &QLineEdit::textChanged, this, &FormConfig::saveConfig);

    ui->txtHtmlName1->setText(AppConfig::HtmlName1);
    connect(ui->txtHtmlName1, &QLineEdit::textChanged, this, &FormConfig::saveConfig);

    ui->txtHtmlName2->setText(AppConfig::HtmlName2);
    connect(ui->txtHtmlName2, &QLineEdit::textChanged, this, &FormConfig::saveConfig);

    ui->ckEnableSimple->setChecked(AppConfig::EnableSimple);
    connect(ui->ckEnableSimple, &QCheckBox::stateChanged, this, &FormConfig::saveConfig);

    ui->ckEnableNetPush->setChecked(AppConfig::EnableNetPush);
    connect(ui->ckEnableNetPush, &QCheckBox::stateChanged, this, &FormConfig::saveConfig);

    ui->ckEnableDevicePush->setChecked(AppConfig::EnableDevicePush);
    connect(ui->ckEnableDevicePush, &QCheckBox::stateChanged, this, &FormConfig::saveConfig);

    ui->ckEnableSpeedPush->setChecked(AppConfig::EnableSpeedPush);
    connect(ui->ckEnableSpeedPush, &QCheckBox::stateChanged, this, &FormConfig::saveConfig);

    ui->ckEnableLoopPush->setChecked(AppConfig::EnableLoopPush);
    connect(ui->ckEnableLoopPush, &QCheckBox::stateChanged, this, &FormConfig::saveConfig);

    ui->ckEnableFilePush->setChecked(AppConfig::EnableFilePush);
    connect(ui->ckEnableFilePush, &QCheckBox::stateChanged, this, &FormConfig::saveConfig);

    ui->ckEnableImagePush->setChecked(AppConfig::EnableImagePush);
    connect(ui->ckEnableImagePush, &QCheckBox::stateChanged, this, &FormConfig::saveConfig);
}

void FormConfig::saveConfig() {
    AppConfig::WindowTitle = ui->txtWindowTitle->text().trimmed();
    AppConfig::OpenGLType = ui->cboxOpenGLType->currentIndex();
    AppConfig::CheckRun = ui->ckCheckRun->isChecked();
    AppConfig::AutoRun = ui->ckAutoRun->isChecked();
    AppConfig::UseTray = ui->ckUseTray->isChecked();
    AppConfig::AutoCopy = ui->ckAutoCopy->isChecked();
    AppConfig::PublicUrl = ui->txtPublicUrl->text().trimmed();
    AppConfig::HttpUrl = ui->txtHttpUrl->text().trimmed();
    AppConfig::HtmlName1 = ui->txtHtmlName1->text().trimmed();
    AppConfig::HtmlName2 = ui->txtHtmlName2->text().trimmed();

    AppConfig::EnableSimple = ui->ckEnableSimple->isChecked();
    AppConfig::EnableNetPush = ui->ckEnableNetPush->isChecked();
    AppConfig::EnableDevicePush = ui->ckEnableDevicePush->isChecked();
    AppConfig::EnableSpeedPush = ui->ckEnableSpeedPush->isChecked();
    AppConfig::EnableLoopPush = ui->ckEnableLoopPush->isChecked();
    AppConfig::EnableFilePush = ui->ckEnableFilePush->isChecked();
    AppConfig::EnableImagePush = ui->ckEnableImagePush->isChecked();
    AppConfig::writeConfig();
}

void FormConfig::on_btnHtmlName1_clicked() {
    QString fileName = QtHelper::getOpenFileName();
    if (!fileName.isEmpty()) {
        ui->txtHtmlName1->setText(fileName);
    }
}

void FormConfig::on_btnHtmlName2_clicked() {
    QString fileName = QtHelper::getOpenFileName();
    if (!fileName.isEmpty()) {
        ui->txtHtmlName2->setText(fileName);
    }
}
