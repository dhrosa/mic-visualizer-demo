#include "./image_viewer.h"

#include <QColor>
#include <QImage>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QRectF>
#include <QTransform>
#include <mutex>

#include "colormaps.h"

ImageViewer::ImageViewer(QSize image_size)
    : image_size_(image_size),
      primary_(image_size, QImage::Format_RGB32),
      cursor_(new Cursor(this)) {
  primary_.fill(0xFF'00'00'00);
  secondary_ = primary_.copy();
  setMouseTracking(true);
  setAttribute(Qt::WA_NoSystemBackground);
  setAttribute(Qt::WA_OpaquePaintEvent);
}

QTransform ImageViewer::logicalToWidgetTransform() const {
  return QTransform::fromScale(
      static_cast<double>(width()) / image_size_.width(),
      static_cast<double>(height()) / image_size_.height());
}

QTransform ImageViewer::widgetToLogicalTransform() const {
  return QTransform::fromScale(
      static_cast<double>(image_size_.width()) / width(),
      static_cast<double>(image_size_.height()) / height());
}

void ImageViewer::UpdateImage(absl::AnyInvocable<void(QImage&) &&> f) {
  QImage* image;
  {
    absl::MutexLock lock(&mutex_);
    image = &secondary_;
  }
  std::move(f)(*image);
  {
    absl::MutexLock lock(&mutex_);
    std::swap(primary_, secondary_);
  }
  update();
}

void ImageViewer::moveEvent(QMoveEvent* event) {
  update();
  QWidget::moveEvent(event);
}

void ImageViewer::enterEvent(QEnterEvent* event) {
  cursor_->show();
  cursor_->raise();
}

void ImageViewer::mouseMoveEvent(QMouseEvent* event) {
  const QPointF widget_pos(event->pos());
  const QPointF image_pos = widgetToLogicalTransform().map(widget_pos);

  // Each image pixel is mapped to a rectangle identified by its top-left
  // corner (towards origin).
  const QPoint snapped_image_pos(image_pos.x(), image_pos.y());
  const QRectF bin_rect(snapped_image_pos, QSize(1, 1));
  emit binHovered(snapped_image_pos);

  cursor_->setTarget(logicalToWidgetTransform().mapRect(bin_rect));
  cursor_->setGeometry(0, 0, width(), height());
  cursor_->update();
}

void ImageViewer::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  const QRect dest_rect = event->rect();
  absl::MutexLock lock(&mutex_);
  const QRect source_rect =
      widgetToLogicalTransform().mapRect(QRectF(dest_rect)).toAlignedRect();
  painter.setWindow(primary_.rect());
  painter.drawImage(source_rect.topLeft(), primary_, source_rect);
}
