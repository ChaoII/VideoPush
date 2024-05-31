#pragma once

#include <QWidget>

class FFmpegThread;

namespace Ui {
    class FormDevicePush;
}

class FormDevicePush : public QWidget {
Q_OBJECT

public:
    explicit FormDevicePush(QWidget *parent = nullptr);

    ~FormDevicePush() override;

private slots:

    //初始化界面数据
    void initForm();

    //加载配置参数
    void initConfig();

    //保存配置参数
    void saveConfig();

    //初始化设备名称
    void initDevice();

    //初始化采集地址
    void initUrl();

    //设置参数
    void initPara();

    //播放成功
    void receivePlayStart(int time);

    //播放结束
    void receivePlayFinish();

    void on_btnStart1_clicked();

    void on_btnStart2_clicked();

private:
    Ui::FormDevicePush *ui;

    //音频采集推流线程
    FFmpegThread *ffmpegThread = nullptr;
};

