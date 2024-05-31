#pragma once

#include <QSlider>

class Slider : public QSlider
{
    Q_OBJECT
public:
    explicit Slider(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *);

signals:
    void clicked();
};


