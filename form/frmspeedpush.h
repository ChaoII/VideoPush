#pragma once

#include <QWidget>

namespace Ui {
class frmSpeedPush;
}

class frmSpeedPush : public QWidget
{
    Q_OBJECT

public:
    explicit frmSpeedPush(QWidget *parent = 0);
    ~frmSpeedPush();

private:
    Ui::frmSpeedPush *ui;

private slots:
    //初始化窗体数据
    void initForm();
    //加载配置参数
    void initConfig();
    //保存配置参数
    void saveConfig();

    //播放成功
    void receivePlayStart(int time);
    //播放结束
    void receivePlayFinsh();
    //播放时长
    void receivePosition(qint64 position);

private slots:
    void on_btnStart_clicked();
    void on_sliderPosition_clicked();
    void on_sliderPosition_sliderMoved(int value);
    void on_ckMuted_stateChanged(int arg1);
};

