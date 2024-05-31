#pragma once

#include <QWidget>

namespace Ui {
    class FormMain;
}

class FormMain : public QWidget {
Q_OBJECT

public:
    explicit FormMain(QWidget *parent = nullptr);

    ~FormMain() override;

protected:
    void showEvent(QShowEvent *) override;

    void changeEvent(QEvent *) override;

private:
    Ui::FormMain *ui;

private slots:

    //初始化窗体数据
    void initForm();

    //加载配置文件
    void initConfig();

    // 保存配置
    void saveConfig();
};

