#pragma once

#include <QDialog>

namespace Ui {
class frmNetAdd;
}

class frmNetAdd : public QDialog
{
    Q_OBJECT

public:
    explicit frmNetAdd(QWidget *parent = 0);
    ~frmNetAdd();

private:
    Ui::frmNetAdd *ui;

private slots:
    //初始化窗体数据
    void initForm();
//生成地址
    void initUrl();
    void on_btnOk_clicked();

signals:
    void addUrl(const QString &flag, const QString &url, bool direct);
};

