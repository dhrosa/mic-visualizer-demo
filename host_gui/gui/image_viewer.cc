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

ImageViewer::ImageViewer(std::size_t width, std::size_t height)
    : image_(width, height, QImage::Format_RGB32), cursor_(new Cursor(this)) {
  image_.fill(0xFF'00'00'00);
  setMouseTracking(true);
}

QTransform ImageViewer::logicalToWidgetTransform() const {
  return QTransform::fromScale(static_cast<double>(width()) / image_.width(),
                               static_cast<double>(height()) / image_.height());
}

QTransform ImageViewer::widgetToLogicalTransform() const {
  return QTransform::fromScale(static_cast<double>(image_.width()) / width(),
                               static_cast<double>(image_.height()) / height());
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
  emit binHovered(snapped_image_pos);

  // Snap cursor to midpoint of pixel rectangle.
  cursor_->setTarget(logicalToWidgetTransform().map(QPointF(snapped_image_pos) +
                                                    QPointF(0.5, 0.5)));
  cursor_->setGeometry(0, 0, width(), height());
  cursor_->update();
}

void ImageViewer::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  const QRect dest_rect = event->rect();
  auto lock = std::unique_lock(image_mutex_);
  const QRect source_rect =
      widgetToLogicalTransform().mapRect(QRectF(dest_rect)).toAlignedRect();
  painter.setWindow(image_.rect());
  painter.drawImage(source_rect.topLeft(), image_, source_rect);
}
