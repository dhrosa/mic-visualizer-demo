#pragma once

#include <QWidget>

class Cursor : public QWidget {
 public:
  Cursor(QWidget* parent);

  void setTarget(QRectF target) { target_ = target; }

  void paintEvent(QPaintEvent* event) override;

 private:
  QRectF target_;
};
