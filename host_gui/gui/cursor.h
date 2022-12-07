#pragma once

#include <QWidget>

class Cursor : public QWidget {
 public:
  Cursor(QWidget* parent);

  void setTarget(QPointF target);

  void paintEvent(QPaintEvent* event) override;

 private:
  QPointF target_;
};
