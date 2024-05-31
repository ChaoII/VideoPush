#pragma once

#include "widgethead.h"

class RgbWidget;

class YuvWidget;

class Nv12Widget;

class BannerWidget;

class AbstractVideoWidget : public QWidget {
Q_OBJECT

    Q_PROPERTY(int bgTextSize READ getBgTextSize WRITE setBgTextSize)
    Q_PROPERTY(QString bgText READ getBgText WRITE setBgText)
    Q_PROPERTY(QColor bgColor READ getBgColor WRITE setBgColor)
    Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)

    Q_PROPERTY(int borderWidth READ getBorderWidth WRITE setBorderWidth)
    Q_PROPERTY(QColor borderColor READ getBorderColor WRITE setBorderColor)
    Q_PROPERTY(QColor focusColor READ getFocusColor WRITE setFocusColor)

    Q_PROPERTY(int bannerBgAlpha READ getBannerBgAlpha WRITE setBannerBgAlpha)
    Q_PROPERTY(QColor bannerBgColor READ getBannerBgColor WRITE setBannerBgColor)
    Q_PROPERTY(QColor bannerTextColor READ getBannerTextColor WRITE setBannerTextColor)
    Q_PROPERTY(QColor bannerPressColor READ getBannerPressColor WRITE setBannerPressColor)

public:
    //录像文件保存路径
    static QString recordPath;
    //截图文件保存路径
    static QString snapPath;

    explicit AbstractVideoWidget(QWidget *parent = nullptr);

    ~AbstractVideoWidget() override;

protected:
    //清空opengl绘制数据
    void clearOpenGLData();

    //显示opengl绘制窗体
    void showOpenGLWidget();

    //隐藏opengl绘制窗体
    void hideOpenGLWidget();

    //显示窗体清除数据
    void showEvent(QShowEvent *) override;

    //关闭窗体释放资源
    void closeEvent(QCloseEvent *) override;

    //尺寸发生变化调整位置
    void resizeEvent(QResizeEvent *) override;

    //鼠标进来显示悬浮条
    void enterEvent(QEventx *) override;

    //鼠标离开隐藏悬浮条
    void leaveEvent(QEvent *) override;

    //处理鼠标拖曳文件和数据
    void dropEvent(QDropEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;

    //绘制边框及图片和标签等
    bool eventFilter(QObject *watched, QEvent *event) override;

    void getPoint(QEvent *event);

    void drawBorder(QPainter *painter);

    void drawBg(QPainter *painter);

    void drawInfo(QPainter *painter);

    void drawImage(QPainter *painter);

protected:

    //数据锁
    QMutex mutex_;
    //是否运行
    bool isRunning_ = false;;
    //是否是共享的线程
    bool isShared_ = false;;
    //是否鼠标按下
    bool isPressed_ = false;;
    //定时器刷新标签信息
    QTimer *timerUpdate_;

    //显示rgb数据控件
    RgbWidget *rgbWidget_;
    //显示yuv数据控件
    YuvWidget *yuvWidget_;
    //显示nv12数据控件
    Nv12Widget *nv12Widget_;

    //句柄控件
    QWidget *hwndWidget_;
    //遮罩控件(绘制标签和图形以及图片)
    QWidget *coverWidget_;
    //悬浮条控件
    BannerWidget *bannerWidget_;

    //截图预览
    bool preview_ = false;
    //显示截图标签
    QLabel *label_ = nullptr;

    //采集到的图片
    QImage image_;
    //显示区域
    QRect imageRect_;

    //视频尺寸
    int videoWidth_ = 0;
    int videoHeight_ = 0;
    //频谱区域高度(easyplayer内核用于底部显示可视化频谱)
    int visualHeight_ = 0;

    //旋转角度
    int rotate_ = -1;
    //只有音频
    bool onlyAudio_ = false;

    //实时码率
    qreal kbps_ = 0;
    //硬件加速类型
    QString hardware_ = "none";

    //窗体参数结构体
    WidgetPara widgetPara_;
    //悬浮条信息结构体
    BannerInfo bannerInfo_;
    //编码参数结构体
    EncodePara encodePara_;

    //标签信息集合
    QList<OsdInfo> listOsd_;
    //图形信息集合
    QList<GraphInfo> listGraph_;

public:
    //获取和设置背景文字字体大小
    int getBgTextSize() const;

    void setBgTextSize(int bgTextSize);

    //获取和设置背景文字
    QString getBgText() const;

    void setBgText(const QString &bgText);

    //获取和设置背景颜色
    QColor getBgColor() const;

    void setBgColor(const QColor &bgColor);

    //获取和设置文字颜色
    QColor getTextColor() const;

    void setTextColor(const QColor &textColor);

    //获取和设置边框大小
    int getBorderWidth() const;

    void setBorderWidth(int borderWidth);

    //获取和设置边框颜色
    QColor getBorderColor() const;

    void setBorderColor(const QColor &borderColor);

    //获取和设置焦点颜色
    QColor getFocusColor() const;

    void setFocusColor(const QColor &focusColor);

    //获取和设置悬浮条是否可见
    bool getBannerEnable() const;

    void setBannerEnable(bool bannerEnable);

    //获取和设置悬浮条背景透明度
    int getBannerBgAlpha() const;

    void setBannerBgAlpha(int bannerBgAlpha);

    //获取和设置悬浮条背景颜色
    QColor getBannerBgColor() const;

    void setBannerBgColor(const QColor &bannerBgColor);

    //获取和设置悬浮条文字颜色
    QColor getBannerTextColor() const;

    void setBannerTextColor(const QColor &bannerTextColor);

    //获取和设置悬浮条按下颜色
    QColor getBannerPressColor() const;

    void setBannerPressColor(const QColor &bannerPressColor);

    //设置唯一标识
    void setVideoFlag(const QString &videoFlag);

    //设置缩放显示模式
    void setScaleMode(const ScaleMode &scaleMode);

    //设置视频显示模式
    void setVideoMode(const VideoMode &videoMode);

    //设置共享解码线程
    void setSharedData(bool sharedData);

    //获取视频宽高
    int getVideoWidth() const;

    int getVideoHeight() const;

    //获取是否运行
    bool getIsRunning() const;

    //获取是否仅音频
    bool getOnlyAudio() const;

    //获取图片区域
    QRect getImageRect() const;

    //获取和设置窗体参数
    WidgetPara getWidgetPara() const;

    void setWidgetPara(const WidgetPara &widgetPara);

    //获取和设置悬浮条哪些信息可见
    BannerInfo getBannerInfo() const;

    void setBannerInfo(const BannerInfo &bannerInfo);

    //获取和设置编码参数
    EncodePara getEncodePara() const;

    void setEncodePara(const EncodePara &encodePara);

    //获取悬浮条信息文本内容
    virtual QString getBannerText() const;

    //开启音频振幅
    virtual void setAudioLevel(bool audioLevel);

    //统计实时码率
    virtual void setRealBitRate(bool realBitRate);

    //获取标签和图形集合
    QList<OsdInfo> getListOsd() const;

    QList<GraphInfo> getListGraph() const;

    //设置OSD标签
    void setOsd(const OsdInfo &osd);

    //添加OSD标签
    void appendOsd(const OsdInfo &osd);

    //移除OSD标签
    void removeOsd(const QString &name);

    //清空OSD标签
    void clearOsd();

    //设置图形
    void setGraph(const GraphInfo &graph);

    //添加图形
    void appendGraph(const GraphInfo &graph);

    //删除图形
    void removeGraph(const QString &name);

    //清空图形
    void clearGraph();

public slots:

    //设置图片(比如人工智能运算后的新图片发回绘制)
    void setImage(const QImage &image);

    //清空图片
    void clear();

    //设置调色板
    void setPalettex(bool enabled);

protected slots:

    //视频实时码率
    void receiveKbps(qreal kbps, int frameRate);

    //播放成功
    void receivePlayStart(int time);

    //播放结束
    void receivePlayFinish();

    //收到一张图片
    void receiveImage(const QImage &image, int time);

    //抓拍一张图片
    void snapImage(const QImage &image, const QString &snapName);

    //尺寸变化
    void receiveSizeChanged();

    //接收一帧图片并绘制
    void receiveFrame(int width, int height, quint8 *dataRGB, int type);

    void receiveFrame(int width, int height, quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 linesizeY,
                      quint32 linesizeU, quint32 linesizeV);

    void receiveFrame(int width, int height, quint8 *dataY, quint8 *dataUV, quint32 linesizeY, quint32 linesizeUV);

signals:

    //按下处像素坐标
    void sigReceivePoint(int type, const QPoint &point);

    //音频数据振幅
    void sigReceiveLevel(qreal leftLevel, qreal rightLevel);

    //视频实时码率
    void sigReceiveKbps(qreal kbps, int frameRate);

    //播放成功
    void sigReceivePlayStart(int time);

    //播放结束
    void sigReceivePlayFinish();

    //接收到拖曳文件
    void sigFileDrag(const QString &url);

    //工具栏按钮单击
    void sigBtnClicked(const QString &btnName);

    //标签信息变化
    void sigOsdChanged();

    //图形信息变化
    void sigGraphChanged();
};

