#include "appinit.h"
#include <QMutex>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QWidget>


AppInit::AppInit(QObject *parent) : QObject(parent) {
}

bool AppInit::eventFilter(QObject *watched, QEvent *event) {
    auto w = (QWidget *) watched;
    if (!w->property("canMove").toBool()) {
        return QObject::eventFilter(watched, event);
    }
    static QPoint mousePoint;
    static bool mousePressed = false;

    int type = event->type();
    auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
    if (type == QEvent::MouseButtonPress) {
        if (mouseEvent->button() == Qt::LeftButton) {
            mousePressed = true;
            mousePoint = mouseEvent->globalPos() - w->pos();
        }
    } else if (type == QEvent::MouseButtonRelease) {
        mousePressed = false;
    } else if (type == QEvent::MouseMove) {
        if (mousePressed) {
            w->move(mouseEvent->globalPos() - mousePoint);
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

void AppInit::start() {
    qApp->installEventFilter(this);
}
