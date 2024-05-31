#include "itemdelegate.h"

ItemDelegate::ItemDelegate(QObject *parent) : QItemDelegate(parent)
{

}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem option2 = option;
    QColor color = index.data(Qt::ForegroundRole).value<QColor>();
    if (color.isValid() && color != option.palette.color(QPalette::WindowText)) {
        option2.palette.setColor(QPalette::HighlightedText, color);
    }

    QItemDelegate::paint(painter, option2, index);
}
