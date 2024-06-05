#include "form/frmmain.h"
#include "core_helper/qthelper.h"
#include "core_helper/customstyle.h"
#include "app/commonkey.h"
#include "core_videopush/videopushhelper.h"
#include <QtWidgets>

void initOther() {
    //限制只能一个实例
    if (AppConfig::CheckRun) {
        QtHelper::checkRun();
    }
    //启用秘钥认证
#if 0
    QString key = QtHelper::appPath() + "/config/key.lic";
    if (!CommonKey::checkLicense(key)) {
        exit(0);
    }
#endif
    //指定地址转唯一标识
    VideoPushHelper::cryptoType = AppConfig::CryptoType;
    //如果标识为空则将首个IP地址作为唯一标识
    QStringList ips = QtHelper::getLocalIPs();
    if (AppConfig::CryptoFlag.isEmpty() && ips.count() > 0) {
        VideoPushHelper::cryptoFlag = ips.first();
    }
    //启动流媒体服务
    QtHelper::start(QtHelper::appPath() + "/server", "mediamtx");
    QtHelper::start(QtHelper::appPath() + "/server", "MediaServer");
    //QtHelper::start("H:/0_dll/bin_push/ABLMediaServer-2024-02-27/WinX64", "ABLMediaServer");
}

int main(int argc, char *argv[]) {
    int openGLType = QtHelper::getIniValue(argv, "OpenGLType", "", "video_push").toInt();

    QtHelper::initOpenGL(openGLType);

    bool use96Dpi = (QtHelper::getIniValue(argv, "Use96Dpi", "", "video_push") == "true");
    QtHelper::initMain(false, use96Dpi);

    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(":/main.png"));
    QtHelper::initAll();

#ifdef __arm__
    AppData::RightWidth = 250;
#else
    AppData::RightWidth = 180;
#endif
    AppConfig::ConfigFile = QtHelper::appPath() + "/video_push.ini";
    AppConfig::readConfig();
    initOther();
    FormMain w;
    w.resize(950, 700);
    //自定义了标题则优先取自定义的
    if (AppConfig::WindowTitle.isEmpty()) {
        w.setWindowTitle(QString("Qt/C++推流综合应用示例 V20240413 "));
    } else {
        w.setWindowTitle(AppConfig::WindowTitle);
    }

    QtHelper::setFormInCenter(&w);
//    QtHelper::showForm(&w);

    //最后隐藏在托盘则隐藏主窗体/否则显示主窗体
    if (AppConfig::UseTray && AppConfig::HideTray) {
        w.hide();
    } else {
        AppConfig::FormMax ? w.showMaximized() : w.show();
    }
    return QApplication::exec();
}
