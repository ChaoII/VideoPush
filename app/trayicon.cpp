
#include "trayicon.h"
#include <QMutex>
#include <QApplication>

TrayIcon::TrayIcon(QObject *parent) : QObject(parent) {
    trayIcon_ = new QSystemTrayIcon(this);
    connect(trayIcon_, &QSystemTrayIcon::activated, this, &TrayIcon::iconIsActivated);
    menu_ = new QMenu;
}

void TrayIcon::iconIsActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick: {
            this->showMainWidget();
            break;
        }
        default:
            break;
    }
}

void TrayIcon::setExitDirect(bool exitDirect) {
    if (this->exitDirect_ != exitDirect) {
        this->exitDirect_ = exitDirect;
    }
}

void TrayIcon::setMainWidget(QWidget *mainWidget) {
    this->mainWidget_ = mainWidget;
    auto actionMenu = new QAction("主界面", this);
    connect(actionMenu, &QAction::triggered, this, &TrayIcon::showMainWidget);
    menu_->addAction(actionMenu);
    if (exitDirect_) {
        auto actionExit = new QAction("退出", this);
        connect(actionExit, &QAction::triggered, this, &TrayIcon::closeAll);
        menu_->addAction(actionExit);
    } else {
        auto actionExit = new QAction("退出", this);
        connect(actionExit, &QAction::triggered, this, &TrayIcon::trayIconExit);
        menu_->addAction(actionExit);
    }
    trayIcon_->setContextMenu(menu_);
}

void TrayIcon::showMainWidget() {
    if (mainWidget_) {
        mainWidget_->showNormal();
        mainWidget_->activateWindow();
    }
}

void TrayIcon::showMessage(const QString &title, const QString &msg, QSystemTrayIcon::MessageIcon icon, int msecs) {
    trayIcon_->showMessage(title, msg, icon, msecs);
}

void TrayIcon::setIcon(const QString &strIcon) {
    trayIcon_->setIcon(QIcon(strIcon));
}

void TrayIcon::setToolTip(const QString &tip) {
    trayIcon_->setToolTip(tip);
}

bool TrayIcon::getVisible() const {
    return trayIcon_->isVisible();
}

void TrayIcon::setVisible(bool visible) {
    trayIcon_->setVisible(visible);
}

void TrayIcon::closeAll() {
    trayIcon_->hide();
    trayIcon_->deleteLater();
    QApplication::quit();
}
