#include "frmnetpush.h"
#include "ui_frmnetpush.h"
#include "itemdelegate.h"
#include "core_helper/qthelper.h"
#include "core_videohelper/urlutil.h"
#include "core_videohelper/urlhelper.h"
#include "core_video/videoutil.h"
#include "core_videopush/videopushurl.h"
#include "core_videohelper/imagelabel.h"
#include "core_videoopengl/openglinclude.h"
#include "core_videobase/widgethelper.h"
#include "core_videopush/netpushserver.h"
#include "core_videoffmpeg/ffmpegthread.h"
#include "core_videoffmpeg/ffmpegsavehelper.h"

frmNetPush::frmNetPush(QWidget *parent) : QWidget(parent), ui(new Ui::frmNetPush)
{
    ui->setupUi(this);
    this->initForm();
    this->initTip();
    this->initPara();
    this->initConfig();
    this->initTable();
}

frmNetPush::~frmNetPush()
{
    if (ui->btnStart->text() == "停止服务") {
        pushServer->stop();
    }

    delete ui;
}

void frmNetPush::resizeEvent(QResizeEvent *)
{
    this->moveVideo();
}

void frmNetPush::initForm()
{
    fileNameUrl = QtHelper::appPath() + "/config/video_push_url.txt";
    ui->widgetRight->setFixedWidth(AppData::RightWidth);
    ui->tabWidget->setCurrentIndex(0);
    connect(&netAdd, SIGNAL(addUrl(QString, QString, bool)), this, SLOT(addUrl(QString, QString, bool)));

    //加载流媒体服务器信息
    VideoPushUrl::initServerInfo(QtHelper::appPath() + "/config/video_push_port.txt");
    ui->cboxPushType->addItems(VideoPushUrl::getPushType());

    width = 1920;
    height = 1080;
    rotate = 0;
    videoThread = NULL;
    ui->sliderPosition->setRange(0, 0);

    //实例化文件推流类并绑定信号槽
    pushServer = new NetPushServer(this);
    connect(pushServer, SIGNAL(pushStart(QString, int, int, int, int, bool)), this, SLOT(pushStart(QString, int, int, int, int, bool)));
    connect(pushServer, SIGNAL(pushChanged(QString, int)), this, SLOT(pushChanged(QString, int)));

    on_ckVideoVisible_stateChanged(AppConfig::NetVideoVisible);
    ui->ckVideoVisible->setChecked(AppConfig::NetVideoVisible);
    ui->ckSoundMuted->setChecked(AppConfig::NetSoundMuted);

    //视频显示控件
    labImage = new ImageLabel(ui->frameVideo);
    yuvWidget = new YuvWidget(ui->frameVideo);

#if 0
    //定时器模拟测试
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(on_btnStart_clicked()));
    timer->start(10000);
#endif

    //定时器显示时间/方便推流桌面的时候看到一个动的画面
    QTimer *timer2 = new QTimer(this);
    connect(timer2, SIGNAL(timeout()), this, SLOT(initTip()));
    timer2->start(1000);
}

void frmNetPush::initTip()
{
    VideoPushHelper::initTip(ui->labTip);
}

void frmNetPush::initPara()
{
    //部分流媒体服务支持rtmp推pcm
    static QStringList types;
    if (types.count() == 0) {
        types /*<< "mediamtx"*/ << "ZLMediaKit" << "ABLMediaServer";
    }

    if (types.contains(AppConfig::NetPushType)) {
        FFmpegSaveHelper::rtmp_pcm = true;
    } else {
        FFmpegSaveHelper::rtmp_pcm = false;
    }

    NetPushClient::checkB = AppConfig::CheckB;
    NetPushClient::recordType = AppConfig::RecordType;
    NetPushClient::encodeVideo = AppConfig::EncodeVideo;
    NetPushClient::encodeVideoRatio = AppConfig::EncodeVideoRatio;
    NetPushClient::encodeVideoScale = AppConfig::EncodeVideoScale;

    //全局标志位/是否需要隐藏视频流地址中的密码
    VideoPushHelper::hideUser = AppConfig::NetHideUser;
}

void frmNetPush::initConfig()
{
    ui->cboxPushType->setCurrentIndex(ui->cboxPushType->findText(AppConfig::NetPushType));
    connect(ui->cboxPushType, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    ui->cboxPushMode->setCurrentIndex(ui->cboxPushMode->findText(AppConfig::NetPushMode));
    connect(ui->cboxPushMode, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    QtHelper::initLocalIPs(ui->cboxPushHost, AppConfig::NetPushHost);
    connect(ui->cboxPushHost->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));
    connect(ui->cboxPushHost, SIGNAL(activated(int)), this, SLOT(initUrl()));

    ui->txtPushUrl->setText(AppConfig::NetPushUrl);
    connect(ui->txtPushUrl, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    ui->cboxFlagType->setCurrentIndex(AppConfig::NetFlagType);
    connect(ui->cboxFlagType, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    ui->txtPushFlag->setText(AppConfig::NetPushFlag);
    connect(ui->txtPushFlag, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    ui->cboxPreviewType->setCurrentIndex(ui->cboxPreviewType->findText(AppConfig::NetPreviewType));
    connect(ui->cboxPreviewType, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    VideoUtil::loadCopyNumber(ui->cboxCopyNumber);
    ui->cboxCopyNumber->setCurrentIndex(ui->cboxCopyNumber->findText(QString::number(AppConfig::NetCopyNumber)));
    connect(ui->cboxCopyNumber, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    VideoUtil::loadEncodeVideo(ui->cboxEncodeVideo);
    ui->cboxEncodeVideo->setCurrentIndex(AppConfig::EncodeVideo);
    connect(ui->cboxEncodeVideo, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    VideoUtil::loadEncodeVideoRatio(ui->cboxEncodeVideoRatio);
    ui->cboxEncodeVideoRatio->setCurrentIndex(ui->cboxEncodeVideoRatio->findData(AppConfig::EncodeVideoRatio));
    connect(ui->cboxEncodeVideoRatio, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    VideoUtil::loadEncodeVideoScale(ui->cboxEncodeVideoScale);
    ui->cboxEncodeVideoScale->lineEdit()->setText(AppConfig::EncodeVideoScale);
    connect(ui->cboxEncodeVideoScale->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    ui->ckCheckB->setChecked(AppConfig::CheckB);
    connect(ui->ckCheckB, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

    ui->ckHideUser->setChecked(AppConfig::NetHideUser);
    connect(ui->ckHideUser, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));
}

void frmNetPush::saveConfig()
{
    QString netPushType = ui->cboxPushType->currentText();
    if (AppConfig::NetPushType != netPushType) {
        AppConfig::NetPushType = netPushType;
        this->initUrl();
    }

    QString netPushMode = ui->cboxPushMode->currentText();
    if (AppConfig::NetPushMode != netPushMode) {
        AppConfig::NetPushMode = netPushMode;
        this->initUrl();
    }

    QString netPushHost = ui->cboxPushHost->lineEdit()->text().trimmed();
    if (AppConfig::NetPushHost != netPushHost) {
        AppConfig::NetPushHost = netPushHost;
        this->initUrl();
    }

    AppConfig::NetPushUrl = ui->txtPushUrl->text().trimmed();
    AppConfig::NetFlagType = ui->cboxFlagType->currentIndex();
    AppConfig::NetPushFlag = ui->txtPushFlag->text().trimmed();
    AppConfig::NetPreviewType = ui->cboxPreviewType->currentText();
    AppConfig::NetCopyNumber = ui->cboxCopyNumber->currentText().toInt();
    AppConfig::EncodeVideo = ui->cboxEncodeVideo->currentIndex();
    AppConfig::EncodeVideoRatio = ui->cboxEncodeVideoRatio->itemData(ui->cboxEncodeVideoRatio->currentIndex()).toFloat();
    AppConfig::EncodeVideoScale = ui->cboxEncodeVideoScale->lineEdit()->text().trimmed();
    AppConfig::CheckB = ui->ckCheckB->isChecked();
    AppConfig::NetHideUser = ui->ckHideUser->isChecked();
    AppConfig::writeConfig();

    //立即应用参数
    this->initPara();
}

void frmNetPush::initUrl()
{
    //自动取出对应流媒体服务器对应推流模式对应网卡的推流地址
    QString url = VideoPushUrl::getPushUrl(AppConfig::NetPushType, AppConfig::NetPushMode, AppConfig::NetPushUrl, AppConfig::NetPushHost);
    UrlHelper::removeDefaultPort(url);
    ui->txtPushUrl->setText(url);
}

void frmNetPush::initTable()
{
    //列名和列宽
    QStringList columnName;
    columnName << "唯一标识" << "分辨率" << "状态" << "视频" << "音频" << "完整地址";
    QList<int> columnWidth;
    columnWidth << 270 << 90 << 60 << 40 << 40 << 100;

    //设置列数和列宽
    int columnCount = columnWidth.count();
    ui->tableWidget->setColumnCount(columnCount);
    for (int i = 0; i < columnCount; ++i) {
        ui->tableWidget->setColumnWidth(i, columnWidth.at(i));
    }

    //设置列名和行高
    ui->tableWidget->setHorizontalHeaderLabels(columnName);
    ui->tableWidget->setItemDelegate(new ItemDelegate);
    QtHelper::initTableView(ui->tableWidget, 25, true);

    //加载历史记录
    VideoPushHelper::readUrl(ui->tableWidget, fileNameUrl, pushServer);
    this->connectBtn();

    if (AppConfig::NetPushStart) {
        QMetaObject::invokeMethod(this, "on_btnStart_clicked", Qt::QueuedConnection);
    }
}

void frmNetPush::writeUrl()
{
    this->connectBtn();
    VideoPushHelper::writeUrl(ui->tableWidget, fileNameUrl);
}

void frmNetPush::connectBtn()
{
    //关联按钮信号槽
    int count = ui->tableWidget->rowCount();
    for (int i = 0; i < count; ++i) {
        QPushButton *btn = (QPushButton *)ui->tableWidget->cellWidget(i, 2);
        if (btn) {
            connect(btn, SIGNAL(clicked(bool)), this, SLOT(clicked()), Qt::UniqueConnection);
        }
    }
}

void frmNetPush::moveVideo()
{
    int width = this->width;
    int height = this->height;
    WidgetHelper::rotateSize(rotate, width, height);
    QRect rect = WidgetHelper::getCenterRect(QSize(width, height), ui->frameVideo->rect());

    labImage->clear();
    yuvWidget->clear();
#ifndef openglx
    labImage->setGeometry(rect);
#else
    yuvWidget->setGeometry(rect);
#endif
}

void frmNetPush::resetVideo()
{
    ui->labDuration->setText("00:00");
    ui->labPosition->setText("00:00");
    ui->sliderPosition->setRange(0, 0);

    //先取消之前的关联
    if (videoThread) {
        videoThread->setPlayAudio(false);
        videoThread->setPushPreview(false);
        disconnect(videoThread, SIGNAL(receivePlayStart(int)), this, SLOT(receivePlayStart(int)));
        disconnect(videoThread, SIGNAL(receivePosition(qint64)), this, SLOT(receivePosition(qint64)));
        disconnect(videoThread, SIGNAL(receiveImage(QImage, int)), this, SLOT(receiveImage(QImage, int)));
        disconnect(videoThread, SIGNAL(receiveFrame(int, int, quint8 *, quint8 *, quint8 *, quint32, quint32, quint32)),
                   this, SLOT(receiveFrame(int, int, quint8 *, quint8 *, quint8 *, quint32, quint32, quint32)));
        videoThread = NULL;
    }

    labImage->clear();
    yuvWidget->clear();
}

void frmNetPush::clearUrl()
{
    ui->txtRtspUrl->clear();
    ui->txtRtmpUrl->clear();
    ui->txtHlsUrl->clear();
    ui->txtWebRtcUrl->clear();
}

void frmNetPush::clicked()
{
    //动态单个启动和停止推流
    QPushButton *btn = (QPushButton *)sender();
    int row = btn->objectName().split("_").at(1).toInt();
    QString mediaUrl = ui->tableWidget->item(row, 5)->data(Qt::UserRole).toString();
    if (btn->text() == "启动") {
        pushServer->start(mediaUrl);
    } else {
        pushServer->stop(mediaUrl);
    }
}

void frmNetPush::pushStart(const QString &mediaUrl, int width, int height, int videoStatus, int audioStatus, bool start)
{
    int count = ui->tableWidget->rowCount();
    for (int i = 0; i < count; ++i) {
        QString url = ui->tableWidget->item(i, 5)->data(Qt::UserRole).toString();
        if (url == mediaUrl) {
            //显示分辨率
            QString size = QString("%1 x %2").arg(width).arg(height);
            ui->tableWidget->item(i, 1)->setText(start ? size : "0 x 0");
            //显示视音频数据状态(0=无输入/1=无输出/2=原数据/3=转码中)
            VideoPushHelper::setStatus(ui->tableWidget->item(i, 3), videoStatus);
            VideoPushHelper::setStatus(ui->tableWidget->item(i, 4), audioStatus);
            //显示推流状态
            VideoPushHelper::setStatus(ui->tableWidget, i, start ? 0 : 3, AppConfig::NetPushStart);
            break;
        }
    }
}

void frmNetPush::pushChanged(const QString &mediaUrl, int state)
{
    int count = ui->tableWidget->rowCount();
    for (int i = 0; i < count; ++i) {
        QString url = ui->tableWidget->item(i, 5)->data(Qt::UserRole).toString();
        if (url == mediaUrl) {
            VideoPushHelper::setStatus(ui->tableWidget, i, state, AppConfig::NetPushStart);
            break;
        }
    }
}

void frmNetPush::receivePlayStart(int time)
{
    //调整视频控件位置
    if (videoThread) {
        width = videoThread->getVideoWidth();
        height = videoThread->getVideoHeight();
        rotate = videoThread->getRotate();
        this->moveVideo();
    }
}

void frmNetPush::receivePosition(qint64 position)
{
    //设置当前进度及已播放时长
    ui->sliderPosition->setValue(position);
    ui->labPosition->setText(QtHelper::getTimeString(position));
}

void frmNetPush::receiveImage(const QImage &image, int time)
{
    if (sender()) {

        labImage->setImage(image, true);
    }
}

void frmNetPush::receiveFrame(int width, int height, quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 linesizeY, quint32 linesizeU, quint32 linesizeV)
{
    //这里要过滤下可能线程刚好结束了但是信号已经到这里
    if (sender()) {
        yuvWidget->updateFrame(width, height, dataY, dataU, dataV, linesizeY, linesizeU, linesizeV);
    }
}

void frmNetPush::addUrl(const QString &flag, const QString &url, bool direct)
{
    //非直接添加则取出对应的标识
    QString name = direct ? flag : VideoPushHelper::getFlag(ui->tableWidget, AppConfig::NetFlagType, AppConfig::NetPushFlag);
    VideoPushHelper::addUrl(ui->tableWidget, url, pushServer, name);
}

void frmNetPush::on_btnStart_clicked()
{
    if (ui->btnStart->text() == "启动服务") {
        //先要检查服务是否正常
        QString url = AppConfig::NetPushUrl;
        QString host = UrlHelper::getUrlHost(url);
        int port = UrlHelper::getUrlPort(url);
        if (!url.startsWith("udp://") && !VideoPushUrl::hostLive(host, port, 3000)) {
            QtHelper::showMessageBoxError("流媒体服务器不可达, 请检查地址是否正常或者服务是否开启!");
        } else {
            pushServer->setPushUrl(AppConfig::NetPushUrl);
            pushServer->start();
            ui->btnStart->setText("停止服务");
            ui->widget->setEnabled(false);
        }
    } else {
        this->resetVideo();
        this->clearUrl();
        pushServer->stop();
        ui->btnStart->setText("启动服务");
        ui->widget->setEnabled(true);
    }

    AppConfig::NetPushStart = (ui->btnStart->text() == "停止服务");
    AppConfig::writeConfig();
}

void frmNetPush::on_btnPreview_clicked()
{
    QString pushUrl = AppConfig::NetPushUrl;
    QString pushType = AppConfig::NetPushType;
    QString pullType = AppConfig::NetPreviewType;
    if (pushType == "EasyDarwin" || pushType == "nginx-rtmp" || pushUrl.startsWith("udp://")) {
        return;
    }

    QStringList urls;
    int count = ui->tableWidget->rowCount();
    for (int row = 0; row < count; ++row) {
        QString flag = ui->tableWidget->item(row, 0)->text();
        QString name = ui->tableWidget->item(row, 5)->data(Qt::UserRole).toString();
        QString url = pushServer->getPushUrl(name);
        VideoPushUrl::replaceUrl(AppConfig::PublicUrl, AppConfig::NetPushHost, url);
        QString host = UrlHelper::getUrlHost(url);

        //重新调整拉流类型
        VideoPushUrl::checkPullType(pushType, pullType);
        urls << VideoPushUrl::getPullUrl(pushUrl, pushType, pullType, host, flag);
        //超过最大数量则不处理
        if (urls.count() == VideoPushUrl::maxCount) {
            break;
        }
    }

    //liveplayer表示采用liveplayer播放器
    bool liveplayer = VideoPushUrl::useLivePlayer(pushType, pullType);
    VideoPushUrl::preview(AppConfig::HttpUrl, AppConfig::HtmlName1, AppConfig::NetCopyNumber, urls, liveplayer);
}

void frmNetPush::on_btnRemove_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        QString flag = ui->tableWidget->item(row, 0)->text();
        pushServer->removeUrl(flag);
        ui->tableWidget->removeRow(row);
        this->writeUrl();
        this->resetVideo();
    }
}

void frmNetPush::on_btnClear_clicked()
{
    if (QtHelper::showMessageBoxQuestion("确定要清空吗? 清空后不能恢复!") == QMessageBox::Yes) {
        pushServer->clearUrl();
        ui->tableWidget->setRowCount(0);
        this->writeUrl();
        this->resetVideo();
        this->clearUrl();
    }
}

void frmNetPush::on_btnAddFile_clicked()
{
    QString filter = "视频文件(*.mp4 *.rmvb *.avi *.asf *.mov *.wmv *.mkv);;音频文件(*.mp3 *.wav *.wma *.aac);;所有文件(*.*)";
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "", "", filter);
    foreach (QString fileName, fileNames) {
        this->addUrl(AppConfig::NetPushFlag, fileName, false);
    }

    this->writeUrl();
}

void frmNetPush::on_btnAddPath_clicked()
{
#if 0
    //批量指定测试地址
    on_btnClear_clicked();
    QStringList fileNames;
    fileNames = UrlUtil::getUrls(UrlUtil::Simple);
    foreach (QString fileName, fileNames) {
        if (!fileName.endsWith(".mp3") && !fileName.contains("47.114.127.78")) {
            this->addUrl(AppConfig::NetPushFlag, fileName, false);
        }
    }
#else
    QString path = QFileDialog::getExistingDirectory(this, "", "");
    if (!path.isEmpty()) {
        //找到目录下的音视频文件
        QStringList filters = QString("*.mp4 *.rmvb *.avi *.asf *.mov *.wmv *.mkv *.mp3 *.wav *.wma *.aac").split(" ");
        QDir dir(path);
        QStringList fileNames = dir.entryList(filters);
        foreach (QString fileName, fileNames) {
            this->addUrl(AppConfig::NetPushFlag, path + "/" + fileName, false);
        }
    }
#endif

    this->writeUrl();
}

void frmNetPush::on_btnAddUrl_clicked()
{
    //add.exec();
    //return;

    bool ok = false;
    QString title = "视频流[rtsp://xxxxx rtmp://xxxxx http://xxxxx] / 摄像头[video=USB Video Device|1280x720|30] / 桌面[desktop]";
    QStringList urls = UrlUtil::getUrls(UrlUtil::Simple);
    QString url = QInputDialog::getItem(this, "输入地址", title, urls, urls.indexOf(AppConfig::NetLastUrl), true, &ok);
    url = url.trimmed();
    if (ok && !url.isEmpty()) {
        this->addUrl(AppConfig::NetPushFlag, url, false);
        AppConfig::NetLastUrl = url;
        AppConfig::writeConfig();
    }

    this->writeUrl();
}

void frmNetPush::on_btnClose_clicked()
{
    this->resetVideo();
}

void frmNetPush::on_sliderPosition_clicked()
{
    int value = ui->sliderPosition->value();
    on_sliderPosition_sliderMoved(value);
}

void frmNetPush::on_sliderPosition_sliderMoved(int value)
{
    if (videoThread) {
        videoThread->setPosition(value);
    }
}

void frmNetPush::on_ckSoundMuted_stateChanged(int arg1)
{
    AppConfig::NetSoundMuted = (arg1 != 0);
    AppConfig::writeConfig();
    if (videoThread) {
        videoThread->setMuted(AppConfig::NetSoundMuted);
    }
}

void frmNetPush::on_ckPlayPause_stateChanged(int arg1)
{
    if (videoThread) {
        if (arg1 != 0) {
            videoThread->pause();
        } else {
            videoThread->next();
        }
    }
}

void frmNetPush::on_ckVideoVisible_stateChanged(int arg1)
{
    AppConfig::NetVideoVisible = (arg1 != 0);
    AppConfig::writeConfig();
    ui->frameVideo->setVisible(AppConfig::NetVideoVisible);
    ui->frameSlider->setVisible(AppConfig::NetVideoVisible);
}

void frmNetPush::on_tableWidget_cellPressed(int row, int column)
{
    //取出拉流地址
    QString flag = ui->tableWidget->item(row, 0)->text();
    QString name = ui->tableWidget->item(row, 5)->data(Qt::UserRole).toString();
    QString url = pushServer->getPushUrl(name);
    VideoPushUrl::replaceUrl(AppConfig::PublicUrl, AppConfig::NetPushHost, url);
    QString pushUrl = AppConfig::NetPushUrl;
    QString pushType = AppConfig::NetPushType;

    //自动填充地址(推流后即可用rtmp/rtsp拉流也可用http拉流)
    QString host = UrlHelper::getUrlHost(url);
    QString url1, url2, url3, url4;
    VideoPushUrl::getPullUrl(pushUrl, pushType, host, flag, url1, url2, url3, url4);

    //默认端口可以去掉
    UrlHelper::removeDefaultPort(url1);
    UrlHelper::removeDefaultPort(url2);
    UrlHelper::removeDefaultPort(url3);
    UrlHelper::removeDefaultPort(url4);

    //填入对应的地址
    ui->txtRtspUrl->setText(url1);
    ui->txtRtmpUrl->setText(url2);
    ui->txtHlsUrl->setText(url3);
    ui->txtWebRtcUrl->setText(url4);

    //自动拷贝到剪切板方便直接粘贴测试使用
    if (AppConfig::AutoCopy) {
        qApp->clipboard()->setText(url);
    }

    //已经是同一个则不用切换
    if (videoThread == pushServer->getClient(name)->getVideoThread()) {
        return;
    }

    //切换前先复位
    this->resetVideo();
    //拿到视频解码线程类
    videoThread = pushServer->getClient(name)->getVideoThread();
    if (!videoThread) {
        return;
    }

    //调整视频控件位置
    receivePlayStart(0);

    //设置播放音频和启动推流预览
    videoThread->clearBuffer(false);
    videoThread->setPlayAudio(true);
    videoThread->setPushPreview(true);
    videoThread->setMuted(AppConfig::NetSoundMuted);

    //获取时长和播放进度并关联进度信号
    if (videoThread->getIsFile()) {
        qint64 duration = videoThread->getDuration();
        qint64 position = videoThread->getPosition();
        ui->sliderPosition->setRange(0, duration);
        ui->labDuration->setText(QtHelper::getTimeString(duration));
        this->receivePosition(position);
    }

    //绑定信号槽显示实时视频
    connect(videoThread, SIGNAL(receivePlayStart(int)), this, SLOT(receivePlayStart(int)), Qt::UniqueConnection);
    connect(videoThread, SIGNAL(receivePosition(qint64)), this, SLOT(receivePosition(qint64)), Qt::UniqueConnection);
    connect(videoThread, SIGNAL(receiveImage(QImage, int)), this, SLOT(receiveImage(QImage, int)), Qt::UniqueConnection);
    connect(videoThread, SIGNAL(receiveFrame(int, int, quint8 *, quint8 *, quint8 *, quint32, quint32, quint32)),
            this, SLOT(receiveFrame(int, int, quint8 *, quint8 *, quint8 *, quint32, quint32, quint32)), Qt::UniqueConnection);
}

void frmNetPush::on_tableWidget_cellDoubleClicked(int row, int column)
{
    //停止服务阶段双击修改推流码
    if (AppConfig::NetPushStart || row < 0) {
        return;
    }

    //获取当前双击行对应推流码
    QString url = ui->tableWidget->item(row, 5)->text();
    QString srcFlag = ui->tableWidget->item(row, 0)->text();
    QString dstFlag = QInputDialog::getText(this, "请输入", "输入唯一标识", QLineEdit::Normal, srcFlag);
    if (!dstFlag.isEmpty() && srcFlag != dstFlag) {
        VideoPushHelper::updateUrl(ui->tableWidget, row, srcFlag, dstFlag, url, pushServer);
        this->writeUrl();
    }
}
