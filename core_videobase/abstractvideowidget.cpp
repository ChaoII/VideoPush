#include "abstractvideowidget.h"
#include "widgethelper.h"
#include "bannerwidget.h"

#ifdef openglx

#include "openglinclude.h"

#endif

QString AbstractVideoWidget::recordPath = "./video";
QString AbstractVideoWidget::snapPath = "./snap";

AbstractVideoWidget::AbstractVideoWidget(QWidget *parent) : QWidget(parent) {
    //设置支持拖放
    this->setAcceptDrops(true);
    //设置强焦点
    this->setFocusPolicy(Qt::StrongFocus);

    //句柄控件对象
    hwndWidget_ = new QWidget(this);
    hwndWidget_->setObjectName("hwndWidget");
    //设置个垂直布局以便句柄模式下放置子控件
    auto layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    hwndWidget_->setLayout(layout);

#ifdef openglx
    //GPU绘制控件依附在句柄控件上
    rgbWidget_ = new RgbWidget(hwndWidget_);
    rgbWidget_->setObjectName("rgbWidget");
    rgbWidget_->setVisible(false);

    yuvWidget_ = new YuvWidget(hwndWidget_);
    yuvWidget_->setObjectName("yuvWidget");
    yuvWidget_->setVisible(false);

    nv12Widget_ = new Nv12Widget(hwndWidget_);
    nv12Widget_->setObjectName("nv12Widget");
    nv12Widget_->setVisible(false);
#endif

    //主绘制对象
    coverWidget_ = new QWidget(this);
    coverWidget_->setObjectName("coverWidget");
    coverWidget_->installEventFilter(this);

    //顶部工具栏(默认隐藏/鼠标进入显示/鼠标离开隐藏)
    bannerWidget_ = new BannerWidget(this);
    bannerWidget_->setObjectName("bannerWidget");
    connect(bannerWidget_, SIGNAL(btnClicked(QString)), this, SIGNAL(sigBtnClicked(QString)));
    bannerWidget_->setVisible(false);

    this->installEventFilter(this);

    //定时器刷新标签信息
    timerUpdate_ = new QTimer(this);
    connect(timerUpdate_, SIGNAL(timeout()), coverWidget_, SLOT(update()));
    timerUpdate_->setInterval(1000);

    //关联码率采集信号用于统计显示实时码率
    connect(this, &AbstractVideoWidget::sigReceiveKbps, this, &AbstractVideoWidget::receiveKbps);
}

AbstractVideoWidget::~AbstractVideoWidget() {

}

void AbstractVideoWidget::clearOpenGLData() {
#ifdef openglx
    if (widgetPara_.videoMode == VideoMode_Opengl) {
        rgbWidget_->clear();
        yuvWidget_->clear();
        nv12Widget_->clear();
    }
#endif
}

void AbstractVideoWidget::showOpenGLWidget() {
#ifdef openglx
    //纯音频不显示
    if (widgetPara_.videoMode == VideoMode_Opengl && !onlyAudio_) {
        if (hardware_ == "rgb") {
            rgbWidget_->setVisible(true);
        } else if (hardware_ == "none") {
            yuvWidget_->setVisible(true);
        } else {
            nv12Widget_->setVisible(true);
        }
    }
#endif
}

void AbstractVideoWidget::hideOpenGLWidget() {
#ifdef openglx
    if (widgetPara_.videoMode == VideoMode_Opengl) {
        rgbWidget_->setVisible(false);
        yuvWidget_->setVisible(false);
        nv12Widget_->setVisible(false);
    }
#endif
}

void AbstractVideoWidget::showEvent(QShowEvent *) {
    this->clearOpenGLData();
}

void AbstractVideoWidget::closeEvent(QCloseEvent *) {
    if (label_) {
        label_->deleteLater();
    }
    if (timerUpdate_->isActive()) {
        timerUpdate_->stop();
    }
}

void AbstractVideoWidget::resizeEvent(QResizeEvent *) {
    //获取合理的尺寸(如果有旋转角度则宽高对调)
    QSize imageSize = QSize(videoWidth_, videoHeight_);
    if (rotate_ == 90 || rotate_ == 270) {
        imageSize = QSize(videoHeight_, videoWidth_);
    }

    //频谱区域高度(easyplayer内核用于底部显示可视化频谱)
    imageSize.setHeight(imageSize.height() + visualHeight_);
    imageRect_ = WidgetHelper::getCenterRect(imageSize, this->rect(), widgetPara_.borderWidth, widgetPara_.scaleMode);
    if (widgetPara_.borderWidth == 0 && widgetPara_.scaleMode == ScaleMode_Fill) {
        imageRect_ = this->rect();
    }

    //句柄控件需要根据分辨率大小来调整尺寸
    hwndWidget_->setGeometry(imageRect_);
    //标签图片窗体永远尺寸一样
    coverWidget_->setGeometry(rect());

#ifdef openglx
    //GPU显示控件依附在句柄控件上所以永远和句柄控件尺寸一样
    if (widgetPara_.videoMode == VideoMode_Opengl) {
        //先要清空再设置尺寸否则可能会遇到崩溃
        //this->clearOpenGLData();
        QRect rect = QRect(0, 0, hwndWidget_->width(), hwndWidget_->height());
        if (hardware_ == "rgb") {
            rgbWidget_->setGeometry(rect);
        } else if (hardware_ == "none") {
            yuvWidget_->setGeometry(rect);
        } else {
            nv12Widget_->setGeometry(rect);
        }
    }
#endif

    //设置悬浮工具栏的位置和宽高
    int size = widgetPara_.bannerSize;
    int offset = widgetPara_.borderWidth / 2;
    int offset2 = offset * 2;
    if (widgetPara_.bannerPosition == BannerPosition_Top) {
        bannerWidget_->setGeometry(offset, offset, this->width() - offset2, size);
    } else if (widgetPara_.bannerPosition == BannerPosition_Bottom) {
        bannerWidget_->setGeometry(offset, this->height() - size - offset, this->width() - offset2, size);
    } else if (widgetPara_.bannerPosition == BannerPosition_Left) {
        bannerWidget_->setGeometry(offset, offset, size, this->height() - offset2);
    } else if (widgetPara_.bannerPosition == BannerPosition_Right) {
        bannerWidget_->setGeometry(this->width() - size - offset, offset, size, this->height() - offset2);
    }
}

void AbstractVideoWidget::enterEvent(QEventx *) {
    //这里可以自行增加判断(是否获取了焦点的或者是否处于预览阶段的才需要显示)
    //if (this->hasFocus()) {}
    if (isRunning_ && widgetPara_.bannerEnable) {
        this->setRealBitRate(bannerInfo_.realBitRate);
        bannerWidget_->setVisible(true);
        bannerWidget_->showInfo(getBannerText());
    }
}

void AbstractVideoWidget::leaveEvent(QEvent *) {
    //这里不用判断其他的反正永远隐藏就对了(防止中途改变过其中的变量导致无法隐藏)
    //增加电子放大期间不隐藏/方便标记当前哪个通道处于电子放大期间
    if (!bannerWidget_->getIsCrop()) {
        this->setRealBitRate(false);
        bannerWidget_->setVisible(false);
    }
}

void AbstractVideoWidget::dropEvent(QDropEvent *event) {
    QString url;
    if (event->mimeData()->hasUrls()) {
        url = event->mimeData()->urls().first().toLocalFile();
    } else if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        auto treeWidget = (QTreeWidget *) event->source();
        if (treeWidget) {
            //过滤父节点(那个一般是NVR)
            QTreeWidgetItem *item = treeWidget->currentItem();
            if (item->parent()) {
                url = item->data(0, Qt::UserRole).toString();
            }
        }
    }

    if (!url.isEmpty()) {
        emit sigFileDrag(url);
    }
}

void AbstractVideoWidget::dragEnterEvent(QDragEnterEvent *event) {
    //拖曳进来的时候先判断下类型(非法类型则不处理)
    if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else if (event->mimeData()->hasFormat("text/uri-list")) {
        event->setDropAction(Qt::LinkAction);
        event->accept();
    } else {
        event->ignore();
    }
}

bool AbstractVideoWidget::eventFilter(QObject *watched, QEvent *event) {
    QEvent::Type type = event->type();
    if (watched == coverWidget_) {
        if (type == QEvent::Paint) {
            //矫正颜色
            if (widgetPara_.borderColor == Qt::transparent) {
                widgetPara_.borderColor = palette().shadow().color();
            }
            if (widgetPara_.textColor == Qt::transparent) {
                widgetPara_.textColor = palette().windowText().color();
            }

            QPainter painter;
            //开始绘制
            painter.begin(coverWidget_);
            //开启抗锯齿
            painter.setRenderHints(QPainter::Antialiasing);

            //绘制边框
            drawBorder(&painter);

            //如果图片不为空则绘制图片否则绘制背景
            if (!image_.isNull()) {
                drawImage(&painter);
            } else {
                drawBg(&painter);
            }
            //绘制标签和图形信息
            drawInfo(&painter);
            //结束绘制
            painter.end();
        }
    } else {
        //限定鼠标左键
        if (type == QEvent::MouseButtonPress) {
            if (qApp->mouseButtons() == Qt::LeftButton) {
                isPressed_ = true;
                this->getPoint(event);
            }
        } else if (type == QEvent::MouseMove) {
            if (isPressed_) {
                this->getPoint(event);
            }
        } else if (type == QEvent::MouseButtonRelease) {
            if (isPressed_) {
                isPressed_ = false;
                this->getPoint(event);
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void AbstractVideoWidget::getPoint(QEvent *event) {
    //正在运行中的视频或者设置了图片
    if ((isRunning_ || !image_.isNull()) && !onlyAudio_) {
        auto e = (QMouseEvent *) event;
        QPoint p = hwndWidget_->mapFromParent(e->pos());
        //过滤在绘图区域里面
        if (hwndWidget_->rect().contains(p)) {
            int w = videoWidth_;
            int h = videoHeight_;
            WidgetHelper::rotateSize(rotate_, w, h);
            int x = (double) p.x() / hwndWidget_->width() * w;
            int y = (double) p.y() / hwndWidget_->height() * h;
            emit sigReceivePoint(event->type(), QPoint(x, y));
        }
    }
}

void AbstractVideoWidget::drawBorder(QPainter *painter) {
    painter->save();

    QPen pen;
    pen.setWidth(widgetPara_.borderWidth);
    pen.setColor(hasFocus() ? widgetPara_.focusColor : widgetPara_.borderColor);
    //没有边框则不绘制边框
    painter->setPen(widgetPara_.borderWidth == 0 ? Qt::NoPen : pen);

    //顺带把背景颜色这里也一并处理(有些场景需要某个控件特殊背景颜色显示)
    if (widgetPara_.bgColor != Qt::transparent) {
        painter->setBrush(widgetPara_.bgColor);
    }

    painter->drawRect(rect());
    painter->restore();
}

void AbstractVideoWidget::drawBg(QPainter *painter) {
    //正在运行阶段以及已经获取到了宽高不用绘制背景
    if (isRunning_ || videoWidth_ != 0) {
        return;
    }

#ifdef openglx
    //GPU控件可见不用绘制背景
    if (rgbWidget_->isVisible() || yuvWidget_->isVisible() || nv12Widget_->isVisible()) {
        return;
    }
#endif

    painter->save();

    //背景图片为空则绘制文字否则绘制背景图片
    if (widgetPara_.bgImage.isNull()) {
        painter->setPen(widgetPara_.textColor);
        QFont font;
        font.setPixelSize(widgetPara_.bgTextSize);
        painter->setFont(font);
        painter->drawText(rect(), Qt::AlignCenter, widgetPara_.bgText);
    } else {
        QRect rect = WidgetHelper::getCenterRect(widgetPara_.bgImage.size(), this->rect());
        painter->drawImage(rect, widgetPara_.bgImage);
    }

    painter->restore();
}

void AbstractVideoWidget::drawInfo(QPainter *painter) {
    //只有音频和句柄模式不用绘制OSD
    if (videoWidth_ == 0 || onlyAudio_ || widgetPara_.videoMode == VideoMode_Hwnd) {
        return;
    }

    //标签位置尽量偏移多一点避免遮挡
    QRect rect = image_.isNull() ? coverWidget_->rect() : image_.rect();
    int borderWidth = widgetPara_.borderWidth + 5;
    rect = QRect(rect.x() + borderWidth, rect.y() + borderWidth, rect.width() - (borderWidth * 2),
                 rect.height() - (borderWidth * 2));

    //将标签信息绘制到遮罩层
    if (widgetPara_.osdDrawMode == DrawMode_Cover) {
        for (auto &osd: listOsd_) {
            if (osd.visible) {
                painter->save();
                WidgetHelper::drawOsd(painter, osd, rect);
                painter->restore();
            }
        }
    }

    //将图形信息绘制到遮罩层
    if (widgetPara_.graphDrawMode == DrawMode_Cover) {
        for (auto &graph: listGraph_) {
            painter->save();
            if (!graph.rect.isEmpty()) {
                WidgetHelper::drawRect(painter, graph.rect, graph.borderWidth, graph.borderColor);
            }
            if (!graph.path.isEmpty()) {
                WidgetHelper::drawPath(painter, graph.path, graph.borderWidth, graph.borderColor);
            }
            if (graph.points.count() > 0) {
                WidgetHelper::drawPoints(painter, graph.points, graph.borderWidth, graph.borderColor);
            }
            painter->restore();
        }
    }
}

void AbstractVideoWidget::drawImage(QPainter *painter) {
    if (image_.isNull()) {
        return;
    }

    //标签位置尽量偏移多一点避免遮挡
    QRect rect = image_.isNull() ? coverWidget_->rect() : image_.rect();
    int borderWidth = widgetPara_.borderWidth + 5;
    rect = QRect(rect.x() + borderWidth, rect.y() + borderWidth, rect.width() - (borderWidth * 2),
                 rect.height() - (borderWidth * 2));

    //将标签信息绘制到图片上
    if (widgetPara_.osdDrawMode == DrawMode_Image) {
        for (auto &osd: listOsd_) {
            if (osd.visible) {
                QPainter painter;
                painter.begin(&image_);
                //painter.setRenderHints(QPainter::Antialiasing);
                WidgetHelper::drawOsd(&painter, osd, rect);
                painter.end();
            }
        }
    }

    //将图形信息绘制到图片上
    if (widgetPara_.graphDrawMode == DrawMode_Image) {
        for (auto &graph: listGraph_) {
            QPainter painter;
            painter.begin(&image_);
            //painter.setRenderHints(QPainter::Antialiasing);
            if (!graph.rect.isEmpty()) {
                WidgetHelper::drawRect(&painter, graph.rect, graph.borderWidth, graph.borderColor);
            }
            if (!graph.path.isEmpty()) {
                WidgetHelper::drawPath(&painter, graph.path, graph.borderWidth, graph.borderColor);
            }
            if (graph.points.count() > 0) {
                WidgetHelper::drawPoints(&painter, graph.points, graph.borderWidth, graph.borderColor);
            }
            painter.end();
        }
    }

    //绘制图片
    painter->save();
    painter->drawImage(imageRect_, image_);
    painter->restore();
}

int AbstractVideoWidget::getBgTextSize() const {
    return widgetPara_.bgTextSize;
}

void AbstractVideoWidget::setBgTextSize(int bgTextSize) {
    if (widgetPara_.bgTextSize != bgTextSize) {
        widgetPara_.bgTextSize = bgTextSize;
        this->update();
    }
}

QString AbstractVideoWidget::getBgText() const {
    return widgetPara_.bgText;
}

void AbstractVideoWidget::setBgText(const QString &bgText) {
    if (widgetPara_.bgText != bgText) {
        widgetPara_.bgText = bgText;
        this->update();
    }
}

QColor AbstractVideoWidget::getBgColor() const {
    return widgetPara_.bgColor;
}

void AbstractVideoWidget::setBgColor(const QColor &bgColor) {
    if (widgetPara_.bgColor != bgColor) {
        widgetPara_.bgColor = bgColor;
        this->update();
    }
}

QColor AbstractVideoWidget::getTextColor() const {
    return widgetPara_.textColor;
}

void AbstractVideoWidget::setTextColor(const QColor &textColor) {
    if (widgetPara_.textColor != textColor) {
        widgetPara_.textColor = textColor;
        this->update();
    }
}

int AbstractVideoWidget::getBorderWidth() const {
    return widgetPara_.borderWidth;
}

void AbstractVideoWidget::setBorderWidth(int borderWidth) {
    if (widgetPara_.borderWidth != borderWidth) {
        widgetPara_.borderWidth = borderWidth;
        this->update();
    }
}

QColor AbstractVideoWidget::getBorderColor() const {
    return widgetPara_.borderColor;
}

void AbstractVideoWidget::setBorderColor(const QColor &borderColor) {
    if (widgetPara_.borderColor != borderColor) {
        widgetPara_.borderColor = borderColor;
        this->update();
    }
}

QColor AbstractVideoWidget::getFocusColor() const {
    return widgetPara_.focusColor;
}

void AbstractVideoWidget::setFocusColor(const QColor &focusColor) {
    if (widgetPara_.focusColor != focusColor) {
        widgetPara_.focusColor = focusColor;
        this->update();
    }
}

bool AbstractVideoWidget::getBannerEnable() const {
    return widgetPara_.bannerEnable;
}

void AbstractVideoWidget::setBannerEnable(bool bannerEnable) {
    widgetPara_.bannerEnable = bannerEnable;
}

int AbstractVideoWidget::getBannerBgAlpha() const {
    return bannerWidget_->getBannerPara().bgAlpha;
}

void AbstractVideoWidget::setBannerBgAlpha(int bannerBgAlpha) {
    BannerPara para = bannerWidget_->getBannerPara();
    if (para.bgAlpha != bannerBgAlpha) {
        para.bgAlpha = bannerBgAlpha;
        bannerWidget_->setBannerPara(para);
        bannerWidget_->initStyle();
    }
}

QColor AbstractVideoWidget::getBannerBgColor() const {
    return bannerWidget_->getBannerPara().bgColor;
}

void AbstractVideoWidget::setBannerBgColor(const QColor &bannerBgColor) {
    BannerPara para = bannerWidget_->getBannerPara();
    if (para.bgColor != bannerBgColor) {
        para.bgColor = bannerBgColor;
        bannerWidget_->setBannerPara(para);
        bannerWidget_->initStyle();
    }
}

QColor AbstractVideoWidget::getBannerTextColor() const {
    return bannerWidget_->getBannerPara().textColor;
}

void AbstractVideoWidget::setBannerTextColor(const QColor &bannerTextColor) {
    BannerPara para = bannerWidget_->getBannerPara();
    if (para.textColor != bannerTextColor) {
        para.textColor = bannerTextColor;
        bannerWidget_->setBannerPara(para);
        bannerWidget_->initStyle();
    }
}

QColor AbstractVideoWidget::getBannerPressColor() const {
    return bannerWidget_->getBannerPara().pressColor;
}

void AbstractVideoWidget::setBannerPressColor(const QColor &bannerPressColor) {
    BannerPara para = bannerWidget_->getBannerPara();
    if (para.pressColor != bannerPressColor) {
        para.pressColor = bannerPressColor;
        bannerWidget_->setBannerPara(para);
        bannerWidget_->initStyle();
    }
}

void AbstractVideoWidget::setVideoFlag(const QString &videoFlag) {
    if (widgetPara_.videoFlag != videoFlag) {
        widgetPara_.videoFlag = videoFlag;
    }
}

void AbstractVideoWidget::setScaleMode(const ScaleMode &scaleMode) {
    if (widgetPara_.scaleMode != scaleMode) {
        widgetPara_.scaleMode = scaleMode;
        //主动调用尺寸变化应用
        this->receiveSizeChanged();
    }
}

void AbstractVideoWidget::setVideoMode(const VideoMode &videoMode) {
    if (widgetPara_.videoMode != videoMode) {
        widgetPara_.videoMode = videoMode;
    }
}

void AbstractVideoWidget::setSharedData(bool sharedData) {
    if (widgetPara_.sharedData != sharedData) {
        widgetPara_.sharedData = sharedData;
    }
}

int AbstractVideoWidget::getVideoWidth() const {
    return this->videoWidth_;
}

int AbstractVideoWidget::getVideoHeight() const {
    return this->videoHeight_;
}

bool AbstractVideoWidget::getIsRunning() const {
    return this->isRunning_;
}

bool AbstractVideoWidget::getOnlyAudio() const {
    return this->onlyAudio_;
}

QRect AbstractVideoWidget::getImageRect() const {
    return this->imageRect_;
}

WidgetPara AbstractVideoWidget::getWidgetPara() const {
    return this->widgetPara_;
}

void AbstractVideoWidget::setWidgetPara(const WidgetPara &widgetPara) {
    //先要获取之前的参数
    BannerPara bannerPara = bannerWidget_->getBannerPara();
    //这里过滤下变化了才需要重新加载按钮
    if (bannerPara.position != widgetPara.bannerPosition) {
        bannerPara.position = widgetPara.bannerPosition;
        bannerWidget_->setBannerPara(bannerPara);
        bannerWidget_->initButton();
    }

    //没有启用悬浮栏则立即隐藏
    if (!widgetPara.bannerEnable) {
        bannerWidget_->setVisible(false);
    }

    //拉伸策略变了或者边框大小变了需要出发尺寸变化
    bool needResize = (this->widgetPara_.scaleMode != widgetPara.scaleMode ||
                       this->widgetPara_.borderWidth != widgetPara.borderWidth ||
                       this->widgetPara_.bannerPosition != widgetPara.bannerPosition);
    this->widgetPara_ = widgetPara;
    this->update();

    //如果需要应用缩放模式还需要主动发个尺寸变化事件
    if (needResize) {
        receiveSizeChanged();
    }
}

BannerInfo AbstractVideoWidget::getBannerInfo() const {
    return this->bannerInfo_;
}

void AbstractVideoWidget::setBannerInfo(const BannerInfo &bannerInfo) {
    this->bannerInfo_ = bannerInfo;
}

EncodePara AbstractVideoWidget::getEncodePara() const {
    return this->encodePara_;
}

void AbstractVideoWidget::setEncodePara(const EncodePara &encodePara) {
    this->encodePara_ = encodePara;
}

QString AbstractVideoWidget::getBannerText() const {
    //悬浮条如果宽度不够则不显示文字信息
    int width = bannerWidget_->width();
    if (width < 200) {
        return {};
    }

    QStringList list;
    if (bannerInfo_.resolution) {
        if (!onlyAudio_ && videoWidth_ > 0) {
            list << QString("分辨率: %1 x %2").arg(videoWidth_).arg(videoHeight_);
        }
    }

    return list.join("  ");
}

void AbstractVideoWidget::setAudioLevel(bool audioLevel) {

}

void AbstractVideoWidget::setRealBitRate(bool realBitRate) {

}

QList<OsdInfo> AbstractVideoWidget::getListOsd() const {
    return this->listOsd_;
}

QList<GraphInfo> AbstractVideoWidget::getListGraph() const {
    return this->listGraph_;
}

void AbstractVideoWidget::setOsd(const OsdInfo &osd) {
    //存在则替换
    bool exist = false;
    for (auto &osdInfo: listOsd_) {
        if (osdInfo.name == osd.name) {
            exist = true;
            osdInfo = osd;
            emit sigOsdChanged();
            return;
        }
    }
    //不存在则插入
    if (!exist) {
        this->appendOsd(osd);
    }
}

void AbstractVideoWidget::appendOsd(const OsdInfo &osd) {
    QMutexLocker locker(&mutex_);
    listOsd_ << osd;
    this->update();
    emit sigOsdChanged();
}

void AbstractVideoWidget::removeOsd(const QString &name) {
    QMutexLocker locker(&mutex_);
    for (int i = 0; i < listOsd_.size(); ++i) {
        if (listOsd_.at(i).name == name) {
            listOsd_.removeAt(i);
            break;
        }
    }

    this->update();
    emit sigOsdChanged();
}

void AbstractVideoWidget::clearOsd() {
    QMutexLocker locker(&mutex_);
    listOsd_.clear();
    this->update();
    emit sigOsdChanged();
}

void AbstractVideoWidget::setGraph(const GraphInfo &graph) {
    //存在则替换
    bool exist = false;
    for (auto &graphInfo: listGraph_) {
        if (graphInfo.name == graph.name) {
            exist = true;
            graphInfo = graph;
            emit sigGraphChanged();
            return;
        }
    }

    //不存在则插入
    if (!exist) {
        this->appendGraph(graph);
    }
}

void AbstractVideoWidget::appendGraph(const GraphInfo &graph) {
    QMutexLocker locker(&mutex_);
    listGraph_ << graph;
    this->update();
    emit sigGraphChanged();
}

void AbstractVideoWidget::removeGraph(const QString &name) {
    QMutexLocker locker(&mutex_);
    for (int i = 0; i < listGraph_.size(); ++i) {
        if (listGraph_.at(i).name == name) {
            listGraph_.removeAt(i);
            break;
        }
    }

    this->update();
    emit sigGraphChanged();
}

void AbstractVideoWidget::clearGraph() {
    QMutexLocker locker(&mutex_);
    listGraph_.clear();
    this->update();
    emit sigGraphChanged();
}

void AbstractVideoWidget::setImage(const QImage &image) {
    if (!image.isNull()) {
        //是否需要触发尺寸变化
        bool sizeChanged = (image.width() != videoWidth_ || image.height() != videoHeight_);
        if (sizeChanged) {
            videoWidth_ = image.width();
            videoHeight_ = image.height();
            this->receiveSizeChanged();
        }

        receiveImage(image, 0);
    }
}

void AbstractVideoWidget::clear() {
    videoWidth_ = 0;
    videoHeight_ = 0;
    listOsd_.clear();
    listGraph_.clear();

    //绘制一张空白图片
    this->image_ = QImage();
    this->update();
}

void AbstractVideoWidget::setPalettex(bool enabled) {
    //和大多数播放器一样填充背景颜色(建议用黑色填充)
    if (widgetPara_.scaleMode != ScaleMode_Fill && widgetPara_.fillColor != Qt::transparent) {
        this->setAutoFillBackground(enabled);
        if (enabled) {
            QPalette palette = this->palette();
            palette.setBrush(QPalette::Window, widgetPara_.fillColor);
            this->setPalette(palette);
        }
    }
}

void AbstractVideoWidget::receiveKbps(qreal kbps, int frameRate) {
    //悬浮条可见而且启用了显示实时码率
    if (bannerWidget_->isVisible() && bannerInfo_.realBitRate) {
        this->kbps_ = kbps;
        bannerWidget_->showInfo(getBannerText());
    }
}

void AbstractVideoWidget::receivePlayStart(int time) {
    //播放开始的时候调整一次尺寸
    this->resizeEvent(nullptr);
    this->update();
    this->setPalettex(true);

    this->hideOpenGLWidget();
    this->showOpenGLWidget();
    this->clearOpenGLData();
    if (widgetPara_.osdDrawMode == DrawMode_Cover) {
        timerUpdate_->start();
    }
}

void AbstractVideoWidget::receivePlayFinish() {
    //分辨率复位下绘制那边需要用到这个值
    videoWidth_ = 0;
    videoHeight_ = 0;
    this->clear();
    this->setPalettex(false);

    bannerWidget_->setVisible(false);
    this->clearOpenGLData();
    this->hideOpenGLWidget();
    if (widgetPara_.osdDrawMode == DrawMode_Cover) {
        timerUpdate_->stop();
    }
}

void AbstractVideoWidget::receiveImage(const QImage &image, int time) {
    //qDebug() << TIMEMS << sender() << image.size() << time;
    this->image_ = widgetPara_.copyImage ? image.copy() : image;
    this->update();
}

void AbstractVideoWidget::snapImage(const QImage &image, const QString &snapName) {
    if (preview_ && !image.isNull()) {
        preview_= false;
        label_ = WidgetHelper::showImage(label_, this, image);
    }
}

void AbstractVideoWidget::receiveSizeChanged() {
    auto event = new QResizeEvent(size(), size());
    QApplication::postEvent(this, event);
}

void AbstractVideoWidget::receiveFrame(int width, int height, quint8 *dataRGB, int type) {
#ifdef openglx
    //这里要过滤下可能线程刚好结束了但是信号已经到这里
    if (sender()) {
        rgbWidget_->updateFrame(width, height, dataRGB, type);
    }
#endif
}

void
AbstractVideoWidget::receiveFrame(int width, int height, quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 linesizeY,
                                  quint32 linesizeU, quint32 linesizeV) {
#ifdef openglx
    //这里要过滤下可能线程刚好结束了但是信号已经到这里
    if (sender()) {
        yuvWidget_->updateFrame(width, height, dataY, dataU, dataV, linesizeY, linesizeU, linesizeV);
    }
#endif
}

void AbstractVideoWidget::receiveFrame(int width, int height, quint8 *dataY, quint8 *dataUV, quint32 linesizeY,
                                       quint32 linesizeUV) {
#ifdef openglx
    //这里要过滤下可能线程刚好结束了但是信号已经到这里
    if (sender()) {
        nv12Widget_->updateFrame(width, height, dataY, dataUV, linesizeY, linesizeUV);
    }
#endif
}
