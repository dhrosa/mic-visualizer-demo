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

  // Horizontal
  painter.drawLine(QLineF(0, target_.y(), width(), target_.y()));
  // Vertical
  painter.drawLine(QLineF(target_.x(), 0, target_.x(), height()));
}
