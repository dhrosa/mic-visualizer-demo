#include "cursor.h"

#include <QPainter>
#include <QPen>

Cursor::Cursor(QWidget* parent) : QWidget(parent) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAttribute(Qt::WA_NoSystemBackground);
  setVisible(false);
}

void Cursor::paintEvent(QPaintEvent* event) {
  QPainter painter(this);

  QPen pen;
  pen.setDashPattern({10, 10});
  pen.setCapStyle(Qt::FlatCap);
  pen.setColor(QColor::fromHslF(0, 0, 0.5, 0.5));
  painter.setPen(pen);

  const QPointF center = target_.center();
  // Horizontal
  painter.drawLine(QLineF(0, center.y(), width(), center.y()));
  // Vertical
  painter.drawLine(QLineF(center.x(), 0, center.x(), height()));

  painter.drawRect(target_);
}
