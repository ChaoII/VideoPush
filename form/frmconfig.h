#pragma once

#include <QWidget>

namespace Ui {
    class FormConfig;
}

class FormConfig : public QWidget {
Q_OBJECT

public:
    explicit FormConfig(QWidget *parent = nullptr);

    ~FormConfig() override;

private:
    Ui::FormConfig *ui;

private slots:

    //初始化窗体数据
    void initForm();

    //加载配置参数
    void initConfig();

    //保存配置参数
    void saveConfig();

private slots:

    void on_btnHtmlName1_clicked();

    void on_btnHtmlName2_clicked();
};

