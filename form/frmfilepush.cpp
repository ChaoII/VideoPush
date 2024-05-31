#include "frmfilepush.h"
#include "ui_frmfilepush.h"
#include "qthelper.h"
#include "appconfig.h"
#include "filepushserver.h"
#include "videopushhelper.h"
#include "videopushurl.h"
#include "videoutil.h"

frmFilePush::frmFilePush(QWidget *parent) : QWidget(parent), ui(new Ui::frmFilePush)
{
    ui->setupUi(this);
    this->initForm();
    this->initConfig();
    this->initTable();
}

frmFilePush::~frmFilePush()
{
    if (ui->btnStart->text() == "停止服务") {
        pushServer->stop();
    }

    delete ui;
}

bool frmFilePush::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->txtDebug->viewport() && event->type() == QEvent::MouseButtonDblClick) {
        ui->txtDebug->clear();
        currentCount = 0;
    }

    return QWidget::eventFilter(watched, event);
}

void frmFilePush::initForm()
{
    currentCount = 0;
    fileNameUrl = QtHelper::appPath() + "/config/video_push_file.txt";
    fileNameIP = QtHelper::appPath() + "/config/video_push_ip.txt";
    ui->frame->setFixedWidth(AppData::RightWidth);
    ui->txtDebug->viewport()->installEventFilter(this);

    VideoPushHelper::writeAddress("192.168.0.1", fileNameIP);
    VideoPushHelper::writeAddress("192.168.0.2", fileNameIP);
    VideoPushHelper::writeAddress("192.168.0.3", fileNameIP);

    //实例化文件推流类并绑定信号槽
    pushServer = new FilePushServer(this);
    connect(pushServer, SIGNAL(sendData(QByteArray)), this, SLOT(sendData(QByteArray)));
    connect(pushServer, SIGNAL(receiveData(QByteArray)), this, SLOT(receiveData(QByteArray)));
    connect(pushServer, SIGNAL(receiveConnection(QString)), this, SLOT(receiveConnection(QString)));
    connect(pushServer, SIGNAL(receiveCount(QList<QString>, QList<int>)), this, SLOT(receiveCount(QList<QString>, QList<int>)));

    on_ckDebugVisible_stateChanged(AppConfig::FileDebugVisible);
    ui->ckDebugVisible->setChecked(AppConfig::FileDebugVisible);
    this->receiveConnection("");
}

void frmFilePush::initConfig()
{
    QtHelper::initLocalIPs(ui->cboxListenIP, AppConfig::FileListenIP);
    connect(ui->cboxListenIP->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    ui->txtListenPort->setText(QString::number(AppConfig::FileListenPort));
    connect(ui->txtListenPort, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    ui->cboxFlagType->setCurrentIndex(AppConfig::FileFlagType);
    connect(ui->cboxFlagType, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    ui->txtPushFlag->setText(AppConfig::FilePushFlag);
    connect(ui->txtPushFlag, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    ui->cboxPlayMode->setCurrentIndex(AppConfig::FilePlayMode);
    connect(ui->cboxPlayMode, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    VideoUtil::loadCopyNumber(ui->cboxCopyNumber);
    ui->cboxCopyNumber->setCurrentIndex(ui->cboxCopyNumber->findText(QString::number(AppConfig::FileCopyNumber)));
    connect(ui->cboxCopyNumber, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    QStringList maxCount;
    maxCount << "1" << "5" << "10" << "50" << "100";
    ui->cboxMaxCount->addItems(maxCount);
    ui->cboxMaxCount->lineEdit()->setText(QString::number(AppConfig::FileMaxCount));
    connect(ui->cboxMaxCount->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));
}

void frmFilePush::saveConfig()
{
    AppConfig::FileListenIP = ui->cboxListenIP->lineEdit()->text().trimmed();
    AppConfig::FileListenPort = ui->txtListenPort->text().trimmed().toInt();
    AppConfig::FileFlagType = ui->cboxFlagType->currentIndex();
    AppConfig::FilePushFlag = ui->txtPushFlag->text().trimmed();
    AppConfig::FilePlayMode = ui->cboxPlayMode->currentIndex();
    AppConfig::FileCopyNumber = ui->cboxCopyNumber->currentText().toInt();
    AppConfig::FileMaxCount = ui->cboxMaxCount->lineEdit()->text().toInt();
    AppConfig::writeConfig();
}

void frmFilePush::initTable()
{
    //列名和列宽
    QStringList columnName;
    columnName << "唯一标识" << "文件大小" << "访问数" << "完整地址";
    QList<int> columnWidth;
    columnWidth << 270 << 90 << 60 << 100;

    //设置列数和列宽
    int columnCount = columnWidth.count();
    ui->tableWidget->setColumnCount(columnCount);
    for (int i = 0; i < columnCount; ++i) {
        ui->tableWidget->setColumnWidth(i, columnWidth.at(i));
    }

    //设置列名和行高
    ui->tableWidget->setHorizontalHeaderLabels(columnName);
    QtHelper::initTableView(ui->tableWidget, 25, true);

    //加载历史记录
    VideoPushHelper::readFile(ui->tableWidget, fileNameUrl, pushServer);

    if (AppConfig::FilePushStart) {
        on_btnStart_clicked();
    }
}

void frmFilePush::writeFile()
{
    VideoPushHelper::writeFile(ui->tableWidget, fileNameUrl);
}

void frmFilePush::appendMsg(quint8 type, const QString &data)
{
    QtHelper::appendMsg(ui->txtDebug, type, data, 50, currentCount, false, false);
}

void frmFilePush::sendData(const QByteArray &data)
{
    appendMsg(0, QString(data));
}

void frmFilePush::receiveData(const QByteArray &data)
{
    appendMsg(1, QString(data));
}

void frmFilePush::receiveConnection(const QString &address)
{
    int count = VideoPushHelper::writeAddress(address, fileNameIP);
    ui->lcdNumberAll->display(count);
}

void frmFilePush::receiveCount(const QList<QString> &names, const QList<int> &counts)
{
    int allCount = 0;
    int count = ui->tableWidget->rowCount();
    for (int i = 0; i < count; ++i) {
        QString name = ui->tableWidget->item(i, 3)->text();
        int index = names.indexOf(name);
        int count = index >= 0 ? counts.at(index) : 0;
        allCount += count;
        ui->tableWidget->item(i, 2)->setText(QString::number(count));
    }

    ui->lcdNumber->display(allCount);
}

void frmFilePush::addFile(const QString &file)
{
    QString flag = VideoPushHelper::getFlag(ui->tableWidget, AppConfig::FileFlagType, AppConfig::FilePushFlag);
    VideoPushHelper::addFile(ui->tableWidget, file, pushServer, flag);
}

void frmFilePush::on_btnStart_clicked()
{
    if (ui->btnStart->text() == "启动服务") {
        pushServer->setPlayMode(AppConfig::FilePlayMode);
        pushServer->setMaxCount(AppConfig::FileMaxCount);
        if (pushServer->start(AppConfig::FileListenIP, AppConfig::FileListenPort)) {
            ui->btnStart->setText("停止服务");
            ui->widget->setEnabled(false);
            ui->txtHttpUrl->setText(pushServer->getHttpUrl(""));
        }
    } else {
        pushServer->stop();
        ui->btnStart->setText("启动服务");
        ui->widget->setEnabled(true);
    }

    AppConfig::FilePushStart = (ui->btnStart->text() == "停止服务");
    AppConfig::writeConfig();
}

void frmFilePush::on_btnPreview_clicked()
{
    QStringList urls;
    int count = ui->tableWidget->rowCount();
    for (int row = 0; row < count; ++row) {
        QString name = ui->tableWidget->item(row, 3)->text();
        QString url = pushServer->getHttpUrl(name);
        VideoPushUrl::replaceUrl(AppConfig::PublicUrl, AppConfig::FileListenIP, url);

        //超过最大数量则不处理
        urls << url;
        if (urls.count() == VideoPushUrl::maxCount) {
            break;
        }
    }

    VideoPushUrl::preview(AppConfig::HttpUrl, AppConfig::HtmlName2, AppConfig::FileCopyNumber, urls, false);
}

void frmFilePush::on_btnRemove_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        QString flag = ui->tableWidget->item(row, 0)->text();
        pushServer->removeFile(flag);
        ui->tableWidget->removeRow(row);
        this->writeFile();
    }
}

void frmFilePush::on_btnClear_clicked()
{
    if (QtHelper::showMessageBoxQuestion("确定要清空吗? 清空后不能恢复!") == QMessageBox::Yes) {
        pushServer->clearFile();
        ui->tableWidget->setRowCount(0);
        ui->txtHttpUrl->clear();
        this->writeFile();
    }
}

void frmFilePush::on_btnAddFile_clicked()
{
    QString filter = "视频文件(*.mp4 *.rmvb *.avi *.asf *.mov *.wmv *.mkv);;音频文件(*.mp3 *.wav *.wma *.aac);;所有文件(*.*)";
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "", "", filter);
    foreach (QString fileName, fileNames) {
        this->addFile(fileName);
    }

    this->writeFile();
}

void frmFilePush::on_btnAddPath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "", "");
    if (!path.isEmpty()) {
        //找到目录下的音视频文件
        QStringList filters = QString("*.mp4 *.rmvb *.avi *.asf *.mov *.wmv *.mkv *.mp3 *.wav *.wma *.aac").split(" ");
        QDir dir(path);
        QStringList fileNames = dir.entryList(filters);
        foreach (QString fileName, fileNames) {
            this->addFile(path + "/" + fileName);
        }
    }

    this->writeFile();
}

void frmFilePush::on_ckDebugVisible_stateChanged(int arg1)
{
    AppConfig::FileDebugVisible = (arg1 != 0);
    AppConfig::writeConfig();
    ui->txtDebug->setVisible(AppConfig::FileDebugVisible);
    ui->gridLayout->setVerticalSpacing(AppConfig::FileDebugVisible ? 6 : 0);
}

void frmFilePush::on_tableWidget_cellPressed(int row, int column)
{
    //取出播放地址
    QString name = ui->tableWidget->item(row, 3)->text();
    QString url = pushServer->getHttpUrl(name);
    VideoPushUrl::replaceUrl(AppConfig::PublicUrl, AppConfig::FileListenIP, url);
    ui->txtHttpUrl->setText(url);

    //自动拷贝到剪切板方便直接粘贴测试使用
    if (AppConfig::AutoCopy) {
        qApp->clipboard()->setText(url);
    }
}

void frmFilePush::on_tableWidget_cellDoubleClicked(int row, int column)
{
    //停止服务阶段双击修改推流码
    if (AppConfig::FilePushStart || row < 0) {
        return;
    }

    //获取当前双击行对应推流码
    QString url = ui->tableWidget->item(row, 3)->text();
    QString srcFlag = ui->tableWidget->item(row, 0)->text();
    QString dstFlag = QInputDialog::getText(this, "请输入", "输入唯一标识", QLineEdit::Normal, srcFlag);
    if (!dstFlag.isEmpty() && srcFlag != dstFlag) {
        VideoPushHelper::updateFile(ui->tableWidget, row, srcFlag, dstFlag, url, pushServer);
        this->writeFile();
    }
}
