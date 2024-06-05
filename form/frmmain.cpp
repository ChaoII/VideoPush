#include "frmmain.h"
#include "qthelper.h"
#include "appevent.h"
#include "trayicon.h"
#include "frmconfig.h"
#include "frmsimple.h"
#include "frmnetpush.h"
#include "frmdevicepush.h"
#include "frmspeedpush.h"
#include "frmlooppush.h"
#include "frmfilepush.h"
#include "frmimagepush.h"
#include "ui_frmmain.h"


FormMain::FormMain(QWidget *parent) : QWidget(parent), ui(new Ui::FormMain) {
    ui->setupUi(this);
    this->initForm();
    this->initConfig();
}

FormMain::~FormMain() {
    //退出的时候保存配置
    AppConfig::FormMax = this->isMaximized();
    AppConfig::writeConfig();
    delete ui;
}

void FormMain::showEvent(QShowEvent *) {
    AppConfig::HideTray = false;
    AppConfig::writeConfig();
}

void FormMain::changeEvent(QEvent *) {
    //启用托盘服务则隐藏主界面
    if (this->windowState() == Qt::WindowMinimized) {
        if (AppConfig::UseTray) {
            this->hide();
            AppConfig::HideTray = true;
            AppConfig::writeConfig();
            TrayIcon::Instance().showMessage("提示", "程序最小化到后台运行!");
        }
    }
}

void FormMain::initForm() {
    //设置托盘图标和主窗体
    if (AppConfig::UseTray) {
        TrayIcon::Instance().setIcon(":/main.png");
        TrayIcon::Instance().setMainWidget(this);
        TrayIcon::Instance().setVisible(true);
    }
    //设置开机启动
    QtHelper::runWithSystem(AppConfig::AutoRun);
    //添加子窗体
    ui->tabWidget->addTab(new FormConfig, "系统设置");
    if (AppConfig::EnableSimple) {
        ui->tabWidget->addTab(new frmSimple, "最简示例");
    }
    if (AppConfig::EnableNetPush) {
        ui->tabWidget->addTab(new frmNetPush, "网络推流");
    }
    if (AppConfig::EnableDevicePush) {
        ui->tabWidget->addTab(new FormDevicePush, "设备推流");
    }
    if (AppConfig::EnableSpeedPush) {
        ui->tabWidget->addTab(new frmSpeedPush, "倍速推流");
    }
    if (AppConfig::EnableLoopPush) {
        ui->tabWidget->addTab(new frmLoopPush, "多路推流");
    }
    if (AppConfig::EnableFilePush) {
        ui->tabWidget->addTab(new frmFilePush, "文件点播");
    }
    if (AppConfig::EnableImagePush) {
        ui->tabWidget->addTab(new frmImagePush, "图片推流");
    }
}

void FormMain::initConfig() {
    ui->tabWidget->setCurrentIndex(AppConfig::TabIndex);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &FormMain::saveConfig);
}

void FormMain::saveConfig() {
    AppConfig::TabIndex = ui->tabWidget->currentIndex();
    AppConfig::writeConfig();
}

