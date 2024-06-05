#pragma once

#include <QItemDelegate>

class ItemDelegate : public QItemDelegate {
Q_OBJECT

public:
    explicit ItemDelegate(QObject *parent = 0);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


