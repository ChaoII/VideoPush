#pragma once

#include <QWidget>
class NetPushServer;

namespace Ui {
class frmSimple;
}

class frmSimple : public QWidget
{
    Q_OBJECT

public:
    explicit frmSimple(QWidget *parent = 0);
    ~frmSimple();

private:
    Ui::frmSimple *ui;

    //网络推流
    NetPushServer *pushServer;

private slots:
    //初始化窗体数据
    void initForm();
    //加载配置参数
    void initConfig();
    //保存配置参数
    void saveConfig();

private slots:
    void on_btnSelect_clicked();
    void on_btnStart_clicked();
};

