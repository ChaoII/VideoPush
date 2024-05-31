#include "audiolevel.h"
#include "qpainter.h"
#include "qdebug.h"

AudioLevel::AudioLevel(QWidget *parent) : QWidget(parent)
{
    level = 0;
    borderWidth = 1;

    QPalette palette = this->palette();
    borderColor = palette.color(QPalette::Shadow);

    //可以自定义颜色/也可以取默认调色板
#if 1
    bgColorStart = palette.color(QPalette::Highlight);
    bgColorMid = palette.color(QPalette::Highlight);
    bgColorEnd = palette.color(QPalette::Highlight);
#else
    bgColorStart = QColor(85, 255, 0);
    bgColorMid = QColor(249, 216, 47);
    bgColorEnd = QColor(249, 107, 24);
#endif
}

void AudioLevel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    bool vertical = (this->height() > this->width());

    //绘制边框
    QPen pen;
    pen.setWidth(borderWidth);
    pen.setColor(borderColor);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(this->rect());

    //方向不同对应渐变区域不同
    QLinearGradient linearGradient;
    if (vertical) {
        linearGradient = QLinearGradient(QPointF(0, height()), QPointF(0, 0));
    } else {
        linearGradient = QLinearGradient(QPointF(0, 0), QPointF(width(), 0));
    }

    //设置渐变颜色/可以按照OBS软件的风格
#if 1
    linearGradient.setColorAt(0.0, bgColorStart);
    linearGradient.setColorAt(0.7, bgColorMid);
    linearGradient.setColorAt(1.0, bgColorEnd);
#else
    linearGradient.setColorAt(0.0, bgColorStart);
    linearGradient.setColorAt(0.70, bgColorStart);
    linearGradient.setColorAt(0.71, bgColorMid);
    linearGradient.setColorAt(0.85, bgColorMid);
    linearGradient.setColorAt(0.86, bgColorEnd);
    linearGradient.setColorAt(1.0, bgColorEnd);
#endif
    painter.setBrush(linearGradient);
    painter.setPen(Qt::NoPen);

    //根据宽高自动绘制
    int offset = borderWidth / 2 + 1;
    if (vertical) {
        painter.drawRect(offset, height() - (height() * level), width() - offset * 2, height() * level);
    } else {
        painter.drawRect(offset, offset, width() * level, height() - offset * 2);
    }
}

void AudioLevel::setLevel(qreal level)
{
    this->level = level;
    this->update();
}

void AudioLevel::clear()
{
    this->setLevel(0);
}
