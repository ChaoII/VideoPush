#pragma once

#include <QWidget>

class FilePushServer;

namespace Ui {
    class frmFilePush;
}

class frmFilePush : public QWidget {
Q_OBJECT

public:
    explicit frmFilePush(QWidget *parent = nullptr);

    ~frmFilePush() override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::frmFilePush *ui;

    //统计打印消息计数
    int currentCount_ = 0;
    //保存名称集合文件名
    QString fileNameUrl_;
    //保存地址集合文件名
    QString fileNameIP_;
    //文件推流服务对象
    FilePushServer *pushServer_ = nullptr;

private slots:

    //初始化窗体数据
    void initForm();

    //加载配置参数
    void initConfig();

    //保存配置参数
    void saveConfig();

    //初始化表格
    void initTable();

    //保存历史记录
    void writeFile();

private slots:

    //添加数据
    void appendMsg(quint8 type, const QString &data);

    //发送数据
    void sendData(const QByteArray &data);

    //接收数据
    void receiveData(const QByteArray &data);

    //收到新的连接请求
    void receiveConnection(const QString &address);

    //统计每个文件对应连接数
    void receiveCount(const QList<QString> &names, const QList<int> &counts);

    //添加文件
    void addFile(const QString &file);

private slots:

    void on_btnStart_clicked();

    void on_btnPreview_clicked();

    void on_btnRemove_clicked();

    void on_btnClear_clicked();

    void on_btnAddFile_clicked();

    void on_btnAddPath_clicked();

    void on_ckDebugVisible_stateChanged(int arg1);

    void on_tableWidget_cellPressed(int row, int column);

    void on_tableWidget_cellDoubleClicked(int row, int column);
};

