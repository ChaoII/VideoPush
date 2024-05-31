#pragma once

#include <QWidget>
#include "frmnetadd.h"

class ImageLabel;
class YuvWidget;
class FFmpegThread;
class NetPushServer;

namespace Ui {
class frmNetPush;
}

class frmNetPush : public QWidget
{
    Q_OBJECT

public:
    explicit frmNetPush(QWidget *parent = 0);
    ~frmNetPush();

protected:
    void resizeEvent(QResizeEvent *);

private:
    Ui::frmNetPush *ui;

    //添加地址窗体
    frmNetAdd netAdd;

    //保存名称集合文件名
    QString fileNameUrl;
    //网络推流服务对象
    NetPushServer *pushServer;

    //视频宽高
    int width, height, rotate;
    //当前选中推流对应视频解码线程
    FFmpegThread *videoThread;

    //实时视频预览窗体
    ImageLabel *labImage;
    YuvWidget *yuvWidget;

private slots:
    //初始化窗体数据
    void initForm();
    //初始化提示信息
    void initTip();
    //初始化服务参数
    void initPara();
    //加载配置参数
    void initConfig();
    //保存配置参数
    void saveConfig();

    //初始化地址
    void initUrl();
    //初始化表格
    void initTable();
    //保存历史记录
    void writeUrl();
    //绑定按钮信号
    void connectBtn();
    //调整视频控件位置
    void moveVideo();
    //复位视频控件
    void resetVideo();
    //清空拉流地址
    void clearUrl();

private slots:
    //按钮单击
    void clicked();
    //推流开始
    void pushStart(const QString &mediaUrl, int width, int height, int videoStatus, int audioStatus, bool start);
    //推流状态变化
    void pushChanged(const QString &mediaUrl, int state);

    //播放成功
    void receivePlayStart(int time);
    //播放时长
    void receivePosition(qint64 position);

    //收到一张图片
    void receiveImage(const QImage &image, int time);
    //收到一帧yuv视频数据
    void receiveFrame(int width, int height, quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 linesizeY, quint32 linesizeU, quint32 linesizeV);

    //添加地址
    void addUrl(const QString &flag, const QString &url, bool direct);

private slots:
    void on_btnStart_clicked();
    void on_btnPreview_clicked();
    void on_btnRemove_clicked();
    void on_btnClear_clicked();

    void on_btnAddFile_clicked();
    void on_btnAddPath_clicked();
    void on_btnAddUrl_clicked();
    void on_btnClose_clicked();

    void on_sliderPosition_clicked();
    void on_sliderPosition_sliderMoved(int value);
    void on_ckSoundMuted_stateChanged(int arg1);
    void on_ckPlayPause_stateChanged(int arg1);
    void on_ckVideoVisible_stateChanged(int arg1);
    void on_tableWidget_cellPressed(int row, int column);
    void on_tableWidget_cellDoubleClicked(int row, int column);
};

