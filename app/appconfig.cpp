#include "appconfig.h"
#include "qthelper.h"

QString AppConfig::ConfigFile = "config.ini";

QString AppConfig::WindowTitle = "";
int AppConfig::OpenGLType = 0;
bool AppConfig::Use96Dpi = true;
int AppConfig::TabIndex = 1;
bool AppConfig::FormMax = false;
bool AppConfig::CheckRun = true;
bool AppConfig::AutoRun = false;
bool AppConfig::UseTray = true;
bool AppConfig::HideTray = false;

bool AppConfig::AutoCopy = false;
int AppConfig::CryptoType = 1;
QString AppConfig::CryptoFlag = "";
QString AppConfig::PublicUrl = "";
QString AppConfig::HttpUrl = "http://127.0.0.1";
QString AppConfig::HtmlName1 = "./config/video_push_url.html";
QString AppConfig::HtmlName2 = "./config/video_push_file.html";

bool AppConfig::EnableSimple = false;
bool AppConfig::EnableNetPush = true;
bool AppConfig::EnableDevicePush = true;
bool AppConfig::EnableSpeedPush = false;
bool AppConfig::EnableLoopPush = false;
bool AppConfig::EnableFilePush = true;
bool AppConfig::EnableImagePush = false;

QString AppConfig::SimpleMediaUrl = "desktop=desktop|800x600|25|0|0";
QString AppConfig::SimplePushUrl = "rtmp://127.0.0.1/stream";

bool AppConfig::NetPushStart = false;
bool AppConfig::NetVideoVisible = true;
bool AppConfig::NetSoundMuted = false;
bool AppConfig::NetHideUser = false;

bool AppConfig::CheckB = true;
int AppConfig::RecordType = 0;
int AppConfig::EncodeVideo = 0;
float AppConfig::EncodeVideoRatio = 1;
QString AppConfig::EncodeVideoScale = "1";

QString AppConfig::NetPushType = "mediamtx";
QString AppConfig::NetPushMode = "rtmp";
QString AppConfig::NetPushHost = "127.0.0.1";
QString AppConfig::NetPushUrl = "rtmp://127.0.0.1/stream";

int AppConfig::NetFlagType = 0;
QString AppConfig::NetPushFlag = "v";
QString AppConfig::NetPreviewType = "ws-flv";
int AppConfig::NetCopyNumber = 1;
QString AppConfig::NetLastUrl = "http://vfx.mtime.cn/Video/2021/11/16/mp4/211116131456748178.mp4";

bool AppConfig::DevicePushStart1 = false;
bool AppConfig::DevicePushStart2 = false;
QString AppConfig::DeviceVideo = "none";
QString AppConfig::DeviceAudio = "none";
QString AppConfig::DeviceMediaUrl1 = "video=";
QString AppConfig::DeviceMediaUrl2 = "audio=";
QString AppConfig::DevicePushUrl1 = "rtsp://127.0.0.1:8554/stream/video";
QString AppConfig::DevicePushUrl2 = "rtsp://127.0.0.1:8554/stream/audio";
bool AppConfig::DevicePushOsd = true;
bool AppConfig::DevicePushMuted = false;

bool AppConfig::SpeedPushStart = false;
QString AppConfig::SpeedMediaUrl = "http://vfx.mtime.cn/Video/2021/11/16/mp4/211116131456748178.mp4";
QString AppConfig::SpeedPushUrl = "rtsp://127.0.0.1:8554/stream/speed";
float AppConfig::SpeedPushValue = 1;
bool AppConfig::SpeedPushMuted = false;

bool AppConfig::LoopPushStart = false;
bool AppConfig::LoopPushEnable1 = true;
bool AppConfig::LoopPushEnable2 = false;
bool AppConfig::LoopPushEnable3 = false;
QString AppConfig::LoopPushUrl1 = "rtmp://127.0.0.1/stream/s1";
QString AppConfig::LoopPushUrl2 = "rtmp://127.0.0.1/stream/s2";
QString AppConfig::LoopPushUrl3 = "rtmp://127.0.0.1/stream/s3";

bool AppConfig::FilePushStart = false;
bool AppConfig::FileDebugVisible = true;
QString AppConfig::FileListenIP = "127.0.0.1";
int AppConfig::FileListenPort = 6900;

int AppConfig::FileFlagType = 0;
QString AppConfig::FilePushFlag = "v";
int AppConfig::FilePlayMode = 0;
int AppConfig::FileCopyNumber = 1;
int AppConfig::FileMaxCount = 100;

bool AppConfig::ImagePushStart = false;
QString AppConfig::ImageListenIP = "127.0.0.1";
int AppConfig::ImageListenPort = 6901;

void AppConfig::readConfig() {
    QSettings set(ConfigFile, QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    set.setIniCodec("utf-8");
#endif

    set.beginGroup("AppConfig");
    WindowTitle = set.value("WindowTitle", WindowTitle).toString();
    OpenGLType = set.value("OpenGLType", OpenGLType).toInt();
    Use96Dpi = set.value("Use96Dpi", Use96Dpi).toBool();
    TabIndex = set.value("TabIndex", TabIndex).toInt();
    FormMax = set.value("FormMax", FormMax).toBool();
    CheckRun = set.value("CheckRun", CheckRun).toBool();
    AutoRun = set.value("AutoRun", AutoRun).toBool();
    UseTray = set.value("UseTray", UseTray).toBool();
    HideTray = set.value("HideTray", HideTray).toBool();
    set.endGroup();

    set.beginGroup("PublishConfig");
    AutoCopy = set.value("AutoCopy", AutoCopy).toBool();
    CryptoType = set.value("CryptoType", CryptoType).toInt();
    CryptoFlag = set.value("CryptoFlag", CryptoFlag).toString();
    PublicUrl = set.value("PublicUrl", PublicUrl).toString();
    HttpUrl = set.value("HttpUrl", HttpUrl).toString();
    HtmlName1 = set.value("HtmlName1", HtmlName1).toString();
    HtmlName2 = set.value("HtmlName2", HtmlName2).toString();
    set.endGroup();

    set.beginGroup("EnableConfig");
    EnableSimple = set.value("EnableSimple", EnableSimple).toBool();
    EnableNetPush = set.value("EnableNetPush", EnableNetPush).toBool();
    EnableDevicePush = set.value("EnableDevicePush", EnableDevicePush).toBool();
    EnableSpeedPush = set.value("EnableSpeedPush", EnableSpeedPush).toBool();
    EnableLoopPush = set.value("EnableLoopPush", EnableLoopPush).toBool();
    EnableFilePush = set.value("EnableFilePush", EnableFilePush).toBool();
    EnableImagePush = set.value("EnableImagePush", EnableImagePush).toBool();
    set.endGroup();

    set.beginGroup("SimpleConfig");
    SimpleMediaUrl = set.value("SimpleMediaUrl", SimpleMediaUrl).toString();
    SimplePushUrl = set.value("SimplePushUrl", SimplePushUrl).toString();
    set.endGroup();

    set.beginGroup("NetPush");
    NetPushStart = set.value("NetPushStart", NetPushStart).toBool();
    NetVideoVisible = set.value("NetVideoVisible", NetVideoVisible).toBool();
    NetSoundMuted = set.value("NetSoundMuted", NetSoundMuted).toBool();
    NetHideUser = set.value("NetHideUser", NetHideUser).toBool();

    CheckB = set.value("CheckB", CheckB).toBool();
    RecordType = set.value("RecordType", RecordType).toInt();
    EncodeVideo = set.value("EncodeVideo", EncodeVideo).toInt();
    EncodeVideoRatio = set.value("EncodeVideoRatio", EncodeVideoRatio).toFloat();
    EncodeVideoScale = set.value("EncodeVideoScale", EncodeVideoScale).toString();

    NetPushType = set.value("NetPushType", NetPushType).toString();
    NetPushMode = set.value("NetPushMode", NetPushMode).toString();
    NetPushHost = set.value("NetPushHost", NetPushHost).toString();
    NetPushUrl = set.value("NetPushUrl", NetPushUrl).toString();

    NetFlagType = set.value("NetFlagType", NetFlagType).toInt();
    NetPushFlag = set.value("NetPushFlag", NetPushFlag).toString();
    NetPreviewType = set.value("NetPreviewType", NetPreviewType).toString();
    NetCopyNumber = set.value("NetCopyNumber", NetCopyNumber).toInt();
    NetLastUrl = set.value("NetLastUrl", NetLastUrl).toString();
    set.endGroup();

    set.beginGroup("DevicePush");
    DevicePushStart1 = set.value("DevicePushStart1", DevicePushStart1).toBool();
    DevicePushStart2 = set.value("DevicePushStart2", DevicePushStart2).toBool();
    DeviceVideo = set.value("DeviceVideo", DeviceVideo).toString();
    DeviceAudio = set.value("DeviceAudio", DeviceAudio).toString();
    DeviceMediaUrl1 = set.value("DeviceMediaUrl1", DeviceMediaUrl1).toString();
    DeviceMediaUrl2 = set.value("DeviceMediaUrl2", DeviceMediaUrl2).toString();
    DevicePushUrl1 = set.value("DevicePushUrl1", DevicePushUrl1).toString();
    DevicePushUrl2 = set.value("DevicePushUrl2", DevicePushUrl2).toString();
    DevicePushOsd = set.value("DevicePushOsd", DevicePushOsd).toBool();
    DevicePushMuted = set.value("DevicePushMuted", DevicePushMuted).toBool();
    set.endGroup();

    set.beginGroup("SpeedPush");
    SpeedPushStart = set.value("SpeedPushStart", SpeedPushStart).toBool();
    SpeedMediaUrl = set.value("SpeedMediaUrl", SpeedMediaUrl).toString();
    SpeedPushUrl = set.value("SpeedPushUrl", SpeedPushUrl).toString();
    SpeedPushValue = set.value("SpeedPushValue", SpeedPushValue).toFloat();
    SpeedPushMuted = set.value("SpeedPushMuted", SpeedPushMuted).toBool();
    set.endGroup();

    set.beginGroup("LoopPush");
    LoopPushStart = set.value("LoopPushStart", LoopPushStart).toBool();
    LoopPushEnable1 = set.value("LoopPushEnable1", LoopPushEnable1).toBool();
    LoopPushEnable2 = set.value("LoopPushEnable2", LoopPushEnable2).toBool();
    LoopPushEnable3 = set.value("LoopPushEnable3", LoopPushEnable3).toBool();
    LoopPushUrl1 = set.value("LoopPushUrl1", LoopPushUrl1).toString();
    LoopPushUrl2 = set.value("LoopPushUrl2", LoopPushUrl2).toString();
    LoopPushUrl3 = set.value("LoopPushUrl3", LoopPushUrl3).toString();
    set.endGroup();

    set.beginGroup("FilePush");
    FilePushStart = set.value("FilePushStart", FilePushStart).toBool();
    FileDebugVisible = set.value("FileDebugVisible", FileDebugVisible).toBool();
    FileListenIP = set.value("FileListenIP", FileListenIP).toString();
    FileListenPort = set.value("FileListenPort", FileListenPort).toInt();

    FileFlagType = set.value("FileFlagType", FileFlagType).toInt();
    FilePushFlag = set.value("FilePushFlag", FilePushFlag).toString();
    FilePlayMode = set.value("FilePlayMode", FilePlayMode).toInt();
    FileCopyNumber = set.value("FileCopyNumber", FileCopyNumber).toInt();
    FileMaxCount = set.value("FileMaxCount", FileMaxCount).toInt();
    set.endGroup();

    set.beginGroup("ImagePush");
    ImagePushStart = set.value("ImagePushStart", ImagePushStart).toBool();
    ImageListenIP = set.value("ImageListenIP", ImageListenIP).toString();
    ImageListenPort = set.value("ImageListenPort", ImageListenPort).toInt();
    set.endGroup();

    if (!QFile(ConfigFile).exists()) {
        writeConfig();
    }
}

void AppConfig::writeConfig() {
    //校验空值/防止空值带来的未知的隐患
    if (HtmlName1.isEmpty()) {
        HtmlName1 = "./config/video_push_url.html";
    }
    if (HtmlName2.isEmpty()) {
        HtmlName2 = "./config/video_push_file.html";
    }

    QSettings set(ConfigFile, QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    set.setIniCodec("utf-8");
#endif

    set.beginGroup("AppConfig");
    set.setValue("WindowTitle", WindowTitle);
    set.setValue("OpenGLType", OpenGLType);
    set.setValue("Use96Dpi", Use96Dpi);
    set.setValue("TabIndex", TabIndex);
    set.setValue("FormMax", FormMax);
    set.setValue("CheckRun", CheckRun);
    set.setValue("AutoRun", AutoRun);
    set.setValue("UseTray", UseTray);
    set.setValue("HideTray", HideTray);
    set.endGroup();

    set.beginGroup("PublishConfig");
    set.setValue("AutoCopy", AutoCopy);
    set.setValue("CryptoType", CryptoType);
    set.setValue("CryptoFlag", CryptoFlag);
    set.setValue("PublicUrl", PublicUrl);
    set.setValue("HttpUrl", HttpUrl);
    set.setValue("HtmlName1", HtmlName1);
    set.setValue("HtmlName2", HtmlName2);
    set.endGroup();

    set.beginGroup("EnableConfig");
    set.setValue("EnableSimple", EnableSimple);
    set.setValue("EnableNetPush", EnableNetPush);
    set.setValue("EnableDevicePush", EnableDevicePush);
    set.setValue("EnableSpeedPush", EnableSpeedPush);
    set.setValue("EnableLoopPush", EnableLoopPush);
    set.setValue("EnableFilePush", EnableFilePush);
    set.setValue("EnableImagePush", EnableImagePush);
    set.endGroup();

    set.beginGroup("SimpleConfig");
    set.setValue("SimpleMediaUrl", SimpleMediaUrl);
    set.setValue("SimplePushUrl", SimplePushUrl);
    set.endGroup();

    set.beginGroup("NetPush");
    set.setValue("NetPushStart", NetPushStart);
    set.setValue("NetVideoVisible", NetVideoVisible);
    set.setValue("NetSoundMuted", NetSoundMuted);
    set.setValue("NetHideUser", NetHideUser);

    set.setValue("CheckB", CheckB);
    set.setValue("RecordType", RecordType);
    set.setValue("EncodeVideo", EncodeVideo);
    set.setValue("EncodeVideoRatio", QString::number(EncodeVideoRatio));
    set.setValue("EncodeVideoScale", EncodeVideoScale);

    set.setValue("NetPushType", NetPushType);
    set.setValue("NetPushMode", NetPushMode);
    set.setValue("NetPushHost", NetPushHost);
    set.setValue("NetPushUrl", NetPushUrl);

    set.setValue("NetFlagType", NetFlagType);
    set.setValue("NetPushFlag", NetPushFlag);
    set.setValue("NetPreviewType", NetPreviewType);
    set.setValue("NetCopyNumber", NetCopyNumber);
    set.setValue("NetLastUrl", NetLastUrl);
    set.endGroup();

    set.beginGroup("DevicePush");
    set.setValue("DevicePushStart1", DevicePushStart1);
    set.setValue("DevicePushStart2", DevicePushStart2);
    set.setValue("DeviceVideo", DeviceVideo);
    set.setValue("DeviceAudio", DeviceAudio);
    set.setValue("DeviceMediaUrl1", DeviceMediaUrl1);
    set.setValue("DeviceMediaUrl2", DeviceMediaUrl2);
    set.setValue("DevicePushUrl1", DevicePushUrl1);
    set.setValue("DevicePushUrl2", DevicePushUrl2);
    set.setValue("DevicePushOsd", DevicePushOsd);
    set.setValue("DevicePushMuted", DevicePushMuted);
    set.endGroup();

    set.beginGroup("SpeedPush");
    set.setValue("SpeedPushStart", SpeedPushStart);
    set.setValue("SpeedMediaUrl", SpeedMediaUrl);
    set.setValue("SpeedPushUrl", SpeedPushUrl);
    set.setValue("SpeedPushValue", QString::number(SpeedPushValue));
    set.setValue("SpeedPushMuted", SpeedPushMuted);
    set.endGroup();

    set.beginGroup("LoopPush");
    set.setValue("LoopPushStart", LoopPushStart);
    set.setValue("LoopPushEnable1", LoopPushEnable1);
    set.setValue("LoopPushEnable2", LoopPushEnable2);
    set.setValue("LoopPushEnable3", LoopPushEnable3);
    set.setValue("LoopPushUrl1", LoopPushUrl1);
    set.setValue("LoopPushUrl2", LoopPushUrl2);
    set.setValue("LoopPushUrl3", LoopPushUrl3);
    set.endGroup();

    set.beginGroup("FilePush");
    set.setValue("FilePushStart", FilePushStart);
    set.setValue("FileDebugVisible", FileDebugVisible);
    set.setValue("FileListenIP", FileListenIP);
    set.setValue("FileListenPort", FileListenPort);

    set.setValue("FileFlagType", FileFlagType);
    set.setValue("FilePushFlag", FilePushFlag);
    set.setValue("FilePlayMode", FilePlayMode);
    set.setValue("FileCopyNumber", FileCopyNumber);
    set.setValue("FileMaxCount", FileMaxCount);
    set.endGroup();

    set.beginGroup("ImagePush");
    set.setValue("ImagePushStart", ImagePushStart);
    set.setValue("ImageListenIP", ImageListenIP);
    set.setValue("ImageListenPort", ImageListenPort);
    set.endGroup();
}
