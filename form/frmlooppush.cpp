#include "frmlooppush.h"
#include "ui_frmlooppush.h"
#include "qthelper.h"
#include "videohelper.h"
#include "videopushhelper.h"
#include "ffmpegthread.h"
#include "ffmpegsave.h"
#include "ffmpeghelper.h"

frmLoopPush::frmLoopPush(QWidget *parent) : QWidget(parent), ui(new Ui::frmLoopPush) {
    ui->setupUi(this);
    this->initForm();
    this->initConfig();
    this->initTable();
}

frmLoopPush::~frmLoopPush() {
    if (ui->btnStart->text() == "停止服务") {
        ffmpegThread->stop();
        ffmpegSave1->stop();
        ffmpegSave2->stop();
        ffmpegSave3->stop();
    }
    delete ui;
}

void frmLoopPush::initForm() {
    rowCount = 0;
    currentRow = 0;
    fileNameUrl = QtHelper::appPath() + "/config/video_push_loop.txt";
    ui->frameRight->setFixedWidth(AppData::RightWidth);

    //实例化核心类
    ffmpegThread = new FFmpegThread(this);
    ffmpegThread->setFlag("采集0");
    //关联信号槽
    connect(ffmpegThread, &FFmpegThread::receivePlayStart, this, &frmLoopPush::receivePlayStart);
    connect(ffmpegThread, &FFmpegThread::receivePlayFinish, this, &frmLoopPush::receivePlayFinish);
    connect(ffmpegThread, &FFmpegThread::receivePlayError, this, &frmLoopPush::receivePlayFinish);

    //设置解码内核
    ffmpegThread->setVideoCore(VideoCore_FFmpeg);
    //设置默认不播放音频
    ffmpegThread->setPlayAudio(false);
    //设置默认不预览视频
    ffmpegThread->setPushPreview(false);
    //设置编码策略和视频压缩比率
    ffmpegThread->setEncodeVideo((EncodeVideo) AppConfig::EncodeVideo);
    ffmpegThread->setEncodeVideoRatio(AppConfig::EncodeVideoRatio);
    ffmpegThread->setEncodeVideoScale(AppConfig::EncodeVideoScale);

    //设置保存视频类将数据包信号发出来用于继续推流
    ffmpegSave = ffmpegThread->getSaveFile();
    connect(ffmpegSave, &FFmpegSave::receivePacket, this, &frmLoopPush::receivePacket);

    //多路推流
    ffmpegSave1 = new FFmpegSave(this);
    ffmpegSave2 = new FFmpegSave(this);
    ffmpegSave3 = new FFmpegSave(this);
    ffmpegSave1->setFlag("地址1");
    ffmpegSave2->setFlag("地址2");
    ffmpegSave3->setFlag("地址3");

    connect(ffmpegSave1, &FFmpegSave::receiveSaveStart, this, &frmLoopPush::receiveSaveStart);
    connect(ffmpegSave2, &FFmpegSave::receiveSaveStart, this, &frmLoopPush::receiveSaveStart);
    connect(ffmpegSave3, &FFmpegSave::receiveSaveStart, this, &frmLoopPush::receiveSaveStart);
    connect(ffmpegSave1, &FFmpegSave::receiveSaveFinish, this, &frmLoopPush::receiveSaveFinish);
    connect(ffmpegSave2, &FFmpegSave::receiveSaveFinish, this, &frmLoopPush::receiveSaveFinish);
    connect(ffmpegSave3, &FFmpegSave::receiveSaveFinish, this, &frmLoopPush::receiveSaveFinish);
}

void frmLoopPush::initConfig() {
    ui->ckPushEnable1->setChecked(AppConfig::LoopPushEnable1);
    connect(ui->ckPushEnable1, &QCheckBox::checkStateChanged, this, &frmLoopPush::saveConfig);

    ui->ckPushEnable2->setChecked(AppConfig::LoopPushEnable2);
    connect(ui->ckPushEnable2, &QCheckBox::checkStateChanged, this, &frmLoopPush::saveConfig);

    ui->ckPushEnable3->setChecked(AppConfig::LoopPushEnable3);
    connect(ui->ckPushEnable3, &QCheckBox::checkStateChanged, this, &frmLoopPush::saveConfig);

    ui->txtPushUrl1->setText(AppConfig::LoopPushUrl1);
    connect(ui->txtPushUrl1, &QTextEdit::textChanged, this, &frmLoopPush::saveConfig);

    ui->txtPushUrl2->setText(AppConfig::LoopPushUrl2);
    connect(ui->txtPushUrl2, &QTextEdit::textChanged, this, &frmLoopPush::saveConfig);

    ui->txtPushUrl3->setText(AppConfig::LoopPushUrl3);
    connect(ui->txtPushUrl3, &QTextEdit::textChanged, this, &frmLoopPush::saveConfig);
}

void frmLoopPush::saveConfig() {
    AppConfig::LoopPushEnable1 = ui->ckPushEnable1->isChecked();
    AppConfig::LoopPushEnable2 = ui->ckPushEnable2->isChecked();
    AppConfig::LoopPushEnable3 = ui->ckPushEnable3->isChecked();
    AppConfig::LoopPushUrl1 = ui->txtPushUrl1->toPlainText().trimmed();
    AppConfig::LoopPushUrl2 = ui->txtPushUrl2->toPlainText().trimmed();
    AppConfig::LoopPushUrl3 = ui->txtPushUrl3->toPlainText().trimmed();
    AppConfig::writeConfig();
}

void frmLoopPush::initTable() {
    //列名和列宽
    QStringList columnName;
    columnName << "时长" << "文件地址( 双击立即切换 )";
    QList<int> columnWidth;
    columnWidth << 100 << 250;

    //设置列数和列宽
    int columnCount = static_cast<int>(columnWidth.count());
    ui->tableWidget->setColumnCount(columnCount);
    for (int i = 0; i < columnCount; ++i) {
        ui->tableWidget->setColumnWidth(i, columnWidth.at(i));
    }

    //设置列名和行高
    ui->tableWidget->setHorizontalHeaderLabels(columnName);
    QtHelper::initTableView(ui->tableWidget, 25, true);

    //加载历史记录
    VideoPushHelper::readLoop(ui->tableWidget, fileNameUrl);
    this->writeFile();

    if (AppConfig::LoopPushStart) {
        on_btnStart_clicked();
    }
}

void frmLoopPush::writeFile() {
    //写入的时候统计总时长
    quint64 duration = VideoPushHelper::writeLoop(ui->tableWidget, fileNameUrl);
    QString min = QString("%1").arg(duration / 60, 2, 10, QChar('0'));
    QString sec = QString("%2").arg(duration % 60, 2, 10, QChar('0'));
    QString time = QString("%1 分 %2 秒").arg(min).arg(sec);
    ui->labTip->setText(QString("总时长: %1").arg(time));
}

void frmLoopPush::receivePlayStart(int time) {
    //打开后才能启动录像(已经设置过仅仅是发送数据包不会真正存储数据到文件)
    QString fileName = QtHelper::appPath() + "/video/push.mp4";
    ffmpegThread->recordStart(fileName);

    //启动其他推流线程(数据复用当前采集线程的保存数据)
    if (ffmpegSave->getIsOk()) {
        MediaType mediaType = ffmpegThread->getMediaType();
        SaveVideoType saveType = SaveVideoType_Mp4;

        //重新编码过的则取视频保存类的对象
        AVStream *videoStreamIn = ffmpegSave->getVideoEncode() ? ffmpegSave->getVideoStream()
                                                               : ffmpegThread->getVideoStream();
        AVStream *audioStreamIn = ffmpegSave->getAudioEncode() ? ffmpegSave->getAudioStream()
                                                               : ffmpegThread->getAudioStream();

        if (AppConfig::LoopPushEnable1) {
            ffmpegSave1->setSavePara(mediaType, saveType, videoStreamIn, audioStreamIn);
            ffmpegSave1->open(AppConfig::LoopPushUrl1);
        }
        if (AppConfig::LoopPushEnable2) {
            ffmpegSave2->setSavePara(mediaType, saveType, videoStreamIn, audioStreamIn);
            ffmpegSave2->open(AppConfig::LoopPushUrl2);
        }
        if (AppConfig::LoopPushEnable3) {
            ffmpegSave3->setSavePara(mediaType, saveType, videoStreamIn, audioStreamIn);
            ffmpegSave3->open(AppConfig::LoopPushUrl3);
        }
    }
}

void frmLoopPush::receivePlayFinish() {
    //停止其他推流线程
    if (ffmpegSave1->getIsOk()) {
        ffmpegSave1->stop();
    }
    if (ffmpegSave2->getIsOk()) {
        ffmpegSave2->stop();
    }
    if (ffmpegSave3->getIsOk()) {
        ffmpegSave3->stop();
    }

    //没有启动服务不用继续
    if (!AppConfig::LoopPushStart) {
        return;
    }

    //逐个播放下去/到了末尾切换到第一个
    if (currentRow < rowCount - 1) {
        currentRow++;
    } else {
        currentRow = 0;
    }

    this->stop();
    qApp->processEvents();
    this->play();
}

void frmLoopPush::receiveSaveStart() {
    auto save = (FFmpegSave *) sender();
    if (save == ffmpegSave1) {
        ui->ckPushEnable1->setEnabled(false);
    } else if (save == ffmpegSave2) {
        ui->ckPushEnable2->setEnabled(false);
    } else if (save == ffmpegSave3) {
        ui->ckPushEnable3->setEnabled(false);
    }
}

void frmLoopPush::receiveSaveFinish() {
    auto save = (FFmpegSave *) sender();
    if (save == ffmpegSave1) {
        ui->ckPushEnable1->setEnabled(true);
    } else if (save == ffmpegSave2) {
        ui->ckPushEnable2->setEnabled(true);
    } else if (save == ffmpegSave3) {
        ui->ckPushEnable3->setEnabled(true);
    }
}

void frmLoopPush::receivePacket(AVPacket *packet) {
    //由于在第二路推流中会被更改所以需要重新拷贝一份
    AVPacket *packet2 = FFmpegHelper::creatPacket(packet);
    AVPacket *packet3 = FFmpegHelper::creatPacket(packet);

    if (AppConfig::LoopPushEnable1 && ffmpegSave1->getIsOk()) {
        ffmpegSave1->writePacket2(packet);
    }

    if (AppConfig::LoopPushEnable2 && ffmpegSave2->getIsOk()) {
        ffmpegSave2->writePacket2(packet2);
    }

    if (AppConfig::LoopPushEnable3 && ffmpegSave3->getIsOk()) {
        ffmpegSave3->writePacket2(packet3);
    }

    //处理完就释放掉
    FFmpegHelper::freePacket(packet);
    FFmpegHelper::freePacket(packet2);
    FFmpegHelper::freePacket(packet3);
}

void frmLoopPush::play() {
    QString url = ui->tableWidget->item(currentRow, 1)->text();
    ui->tableWidget->setCurrentCell(currentRow, 1);
    //要重新设置启用发送数据包(关闭的时候会复位该标志位)
    ffmpegSave->setSendPacket(true, true);
    ffmpegThread->setMediaUrl(url);
    VideoHelper::initVideoPara(ffmpegThread, url);
    ffmpegThread->play();
}

void frmLoopPush::stop() {
    ffmpegThread->stop();
}

void frmLoopPush::setEnable(bool enable) {
    ui->btnAdd->setEnabled(enable);
    ui->btnRemove->setEnabled(enable);
    ui->btnClear->setEnabled(enable);
}

void frmLoopPush::on_btnStart_clicked() {
    if (ui->btnStart->text() == "启动服务") {
        rowCount = ui->tableWidget->rowCount();
        if (rowCount == 0) {
            return;
        }

        currentRow = 0;
        this->play();
        this->setEnable(false);
        ui->btnStart->setText("停止服务");
    } else {
        this->stop();
        this->setEnable(true);
        ui->btnStart->setText("启动服务");
    }

    AppConfig::LoopPushStart = (ui->btnStart->text() == "停止服务");
    AppConfig::writeConfig();
}

void frmLoopPush::on_btnAdd_clicked() {
    QString filter = "视频文件(*.mp4 *.asf);;所有文件(*.*)";
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "", "", filter);
    for (auto &fileName: fileNames) {
        VideoPushHelper::addLoop(ui->tableWidget, fileName);
    }
    this->writeFile();
}

void frmLoopPush::on_btnRemove_clicked() {
    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        ui->tableWidget->removeRow(row);
        this->writeFile();
    }
}

void frmLoopPush::on_btnClear_clicked() {
    if (QtHelper::showMessageBoxQuestion("确定要清空吗? 清空后不能恢复!") == QMessageBox::Yes) {
        ui->tableWidget->setRowCount(0);
        this->writeFile();
    }
}

void frmLoopPush::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item) {
    currentRow = item->row() - 2;
    receivePlayFinish();
}
