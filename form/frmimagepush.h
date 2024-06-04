#pragma once

#include <QWidget>

class ImagePushServer;

namespace Ui {
    class frmImagePush;
}

class frmImagePush : public QWidget {
Q_OBJECT

public:
    explicit frmImagePush(QWidget *parent = nullptr);

    ~frmImagePush() override;

private:
    Ui::frmImagePush *ui;

    //图片推流服务
    ImagePushServer *pushServer_ = nullptr;

private slots:

    //初始化窗体数据
    void initForm();

    //加载配置文件
    void initConfig();

    //保存配置文件
    void saveConfig();

    void test();

private slots:

    //连接数量变化
    void receiveCount(int count);

private slots:

    void on_btnStart_clicked();

    void on_btnRemove_clicked();

    void on_btnClear_clicked();

    void on_btnSendImage_clicked();

    void on_btnOpenCamera_clicked();
};

