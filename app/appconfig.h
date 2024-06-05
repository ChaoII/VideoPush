#pragma once

#include "head.h"

class AppConfig {
public:
    static QString ConfigFile;      //配置文件文件路径及名称

    //全局配置参数
    static QString WindowTitle;     //软件标题
    static int OpenGLType;          //opengl类型
    static bool Use96Dpi;           //开启后禁用分辨率缩放
    static int TabIndex;            //选项卡索引
    static bool FormMax;            //窗体最大化
    static bool CheckRun;           //只允许一个实例
    static bool AutoRun;            //开机启动服务
    static bool UseTray;            //启动托盘服务
    static bool HideTray;           //隐藏程序托盘

    //发布配置参数
    static bool AutoCopy;           //自动复制地址
    static int CryptoType;          //地址加密类型
    static QString CryptoFlag;      //地址加密标识
    static QString PublicUrl;       //外网地址/云服务器内网地址替换成外网地址
    static QString HttpUrl;         //网络服务地址/本地发布网站后的地址
    static QString HtmlName1;       //网络推流保存网页地址
    static QString HtmlName2;       //文件点播保存网页地址

    //启用模块配置参数
    static bool EnableSimple;       //启用简单示例模块
    static bool EnableNetPush;      //启用网络推流模块
    static bool EnableDevicePush;   //启用设备推流模块
    static bool EnableSpeedPush;    //启用倍速推流模块
    static bool EnableLoopPush;     //启用多路推流模块
    static bool EnableFilePush;     //启用文件推流模块
    static bool EnableImagePush;    //启用图片推流模块

    //最简示例配置参数
    static QString SimpleMediaUrl;  //媒体地址
    static QString SimplePushUrl;   //推流地址    

    //网络推流配置参数
    static bool NetPushStart;       //启动服务
    static bool NetVideoVisible;    //视频预览可见
    static bool NetSoundMuted;      //声音静音
    static bool NetHideUser;        //隐藏用户信息

    static bool CheckB;             //是否检查B帧
    static int RecordType;          //录像存储策略
    static int EncodeVideo;         //视频编码格式
    static float EncodeVideoRatio;  //视频压缩比率
    static QString EncodeVideoScale;//视频缩放比例

    static QString NetPushType;     //推流类型
    static QString NetPushMode;     //推流模式
    static QString NetPushHost;     //推流主机
    static QString NetPushUrl;      //推流地址

    static int NetFlagType;         //标识策略
    static QString NetPushFlag;     //推流标识
    static QString NetPreviewType;  //网页预览类型 m3u8/flv/ws-flv/webrtc
    static int NetCopyNumber;       //预览通道复制数量
    static QString NetLastUrl;      //最后选择的地址

    //设备推流配置参数
    static bool DevicePushStart1;   //启动服务1
    static bool DevicePushStart2;   //启动服务2
    static QString DeviceVideo;     //视频设备
    static QString DeviceAudio;     //音频设备
    static QString DeviceMediaUrl1; //媒体地址1
    static QString DeviceMediaUrl2; //媒体地址2
    static QString DevicePushUrl1;  //推流地址1
    static QString DevicePushUrl2;  //推流地址2
    static bool DevicePushOsd;      //带上水印
    static bool DevicePushMuted;    //静音播放

    //倍速推流配置参数
    static bool SpeedPushStart;     //启动服务
    static QString SpeedMediaUrl;   //播放地址
    static QString SpeedPushUrl;    //推流地址
    static float SpeedPushValue;    //推流倍速
    static bool SpeedPushMuted;     //静音播放

    //循环推流配置参数
    static bool LoopPushStart;      //启动服务
    static bool LoopPushEnable1;    //启用推流1
    static bool LoopPushEnable2;    //启用推流2
    static bool LoopPushEnable3;    //启用推流3
    static QString LoopPushUrl1;    //推流地址1
    static QString LoopPushUrl2;    //推流地址2
    static QString LoopPushUrl3;    //推流地址3

    //文件点播配置参数
    static bool FilePushStart;      //启动服务
    static bool FileDebugVisible;   //打印输出可见
    static QString FileListenIP;    //监听网卡
    static int FileListenPort;      //监听端口

    static int FileFlagType;        //标识策略
    static QString FilePushFlag;    //推流标识
    static int FilePlayMode;        //播放模式
    static int FileCopyNumber;      //预览通道复制数量
    static int FileMaxCount;        //最大请求

    //图片推流配置参数
    static bool ImagePushStart;     //启动服务
    static QString ImageListenIP;   //监听网卡
    static int ImageListenPort;     //监听端口

    //读写配置文件
    static void readConfig();       //读取配置文件(在main函数最开始加载程序载入)
    static void writeConfig();      //写入配置文件(在更改配置文件程序关闭时调用)
};

