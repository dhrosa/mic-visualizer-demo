#include "scroll_area.h"

#include <QKeySequence>
#include <QShortcut>

ScrollArea::ScrollArea(QWidget* parent) : QScrollArea(parent) {
  SetFitToWindow(true);
  new QShortcut(QKeySequence::ZoomIn, this, [this] { Zoom(2.0); });
  new QShortcut(QKeySequence::ZoomOut, this, [this] { Zoom(0.5); });
}

void ScrollArea::SetFitToWindow(bool fit) {
  fit_to_window_ = fit;
  setWidgetResizable(fit_to_window_);
  setSizeAdjustPolicy(fit_to_window_ ? QScrollArea::AdjustToContents
                                     : QScrollArea::AdjustIgnored);
}

void ScrollArea::Zoom(double factor) {
  scale_ *= factor;
  widget()->resize(widget()->sizeHint() * scale_);
}
