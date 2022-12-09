#pragma once

#include <QScrollArea>

class ScrollArea : public QScrollArea {
 public:
  ScrollArea(QWidget* parent = nullptr);

  void SetFitToWindow(bool fit);

 private:
  void Zoom(double factor);

  bool fit_to_window_;
  double scale_ = 1.0;
};
