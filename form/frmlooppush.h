#pragma once

#include <QWidget>
#include "core_videoffmpeg/ffmpeginclude.h"
class FFmpegThread;
class FFmpegSave;
class QTableWidgetItem;

namespace Ui {
class frmLoopPush;
}

class frmLoopPush : public QWidget
{
    Q_OBJECT

public:
    explicit frmLoopPush(QWidget *parent = 0);
    ~frmLoopPush();

private:
    Ui::frmLoopPush *ui;

    //总行数和当前行
    int rowCount;
    int currentRow;
    //保存名称集合文件名
    QString fileNameUrl;

    //视频采集线程
    FFmpegThread *ffmpegThread;
    //视频保存线程
    FFmpegSave *ffmpegSave;
    FFmpegSave *ffmpegSave1;
    FFmpegSave *ffmpegSave2;
    FFmpegSave *ffmpegSave3;

private slots:
    //初始化窗体数据
    void initForm();
    //加载配置参数
    void initConfig();
    //保存配置参数
    void saveConfig();

    //初始化表格
    void initTable();
    //保存历史记录
    void writeFile();

    //播放成功
    void receivePlayStart(int time);
    //播放结束
    void receivePlayFinsh();

    //保存成功
    void receiveSaveStart();
    //保存结束
    void receiveSaveFinsh();

    //收到数据包(一般用于多路存储或推流)
    void receivePacket(AVPacket *packet);

private slots:
    void play();
    void stop();
    void setEnable(bool enable);
    void on_btnStart_clicked();

    void on_btnAdd_clicked();
    void on_btnRemove_clicked();
    void on_btnClear_clicked();
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);
};

