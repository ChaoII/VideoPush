#pragma once

#include <QWidget>

class NetPushServer;

namespace Ui {
    class frmSimple;
}

class frmSimple : public QWidget {
Q_OBJECT

public:
    explicit frmSimple(QWidget *parent = nullptr);

    ~frmSimple() override;

private:
    Ui::frmSimple *ui;

    //网络推流
    NetPushServer *pushServer_ = nullptr;

private slots:

    //加载配置参数
    void initConfig();

    //保存配置参数
    void saveConfig();

private slots:

    void on_btnSelect_clicked();

    void on_btnStart_clicked();
};

