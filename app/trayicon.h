#pragma once

/**
 * 1. 可设置托盘图标对应所属主窗体。
 * 2. 可设置托盘图标。
 * 3. 可设置提示信息。
 * 4. 自带右键菜单。
 * 5. 采用了最新的c++ 11的单例类的实现方式
 */

#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>

class TrayIcon : public QObject {
Q_OBJECT

public:
    static TrayIcon &Instance() {
        static TrayIcon trayIcon;
        return trayIcon;
    }

private:
    explicit TrayIcon(QObject *parent = nullptr);

    ~TrayIcon() override = default;

    TrayIcon(const TrayIcon &);

    TrayIcon &operator=(const TrayIcon &);

private:
    QWidget *mainWidget_ = nullptr;            //对应所属主窗体
    QSystemTrayIcon *trayIcon_ = nullptr;      //托盘对象
    QMenu *menu_ = nullptr;                    //右键菜单
    bool exitDirect_ = true;                //是否直接退出

private slots:

    void iconIsActivated(QSystemTrayIcon::ActivationReason reason);

public:
    //设置是否直接退出,如果不是直接退出则发送信号给主界面
    void setExitDirect(bool exitDirect);

    //设置所属主窗体
    void setMainWidget(QWidget *mainWidget);

    //显示消息
    void showMessage(const QString &title, const QString &msg,
                     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information, int msecs = 5000);

    //设置图标
    void setIcon(const QString &strIcon);

    //设置提示信息
    void setToolTip(const QString &tip);

    //获取和设置是否可见
    bool getVisible() const;

    void setVisible(bool visible);

public slots:

    //退出所有
    void closeAll();

    //显示主窗体
    void showMainWidget();

signals:

    void trayIconExit();
};

