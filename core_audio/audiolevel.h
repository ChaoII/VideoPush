#pragma once

#include <QWidget>

class AudioLevel : public QWidget {
Q_OBJECT

public:
    explicit AudioLevel(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);

private:
    //当前振幅
    qreal level;

    //边框大小和颜色
    int borderWidth;
    QColor borderColor;

    //背景颜色
    QColor bgColorStart;
    QColor bgColorMid;
    QColor bgColorEnd;

public slots:

    void setLevel(qreal level);

    void clear();
};

