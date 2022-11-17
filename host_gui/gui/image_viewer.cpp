#include "./image_viewer.h"

#include <QColor>
#include <QImage>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QRectF>
#include <QTransform>
#include <mutex>

ImageViewer::ImageViewer() : image_(1920, 1080, QImage::Format_ARGB32) {
  image_.fill(QColor::fromHslF(0.75, 0.5, 0.5));
}

QTransform ImageViewer::logicalToWidgetTransform() const {
  return QTransform::fromScale(static_cast<double>(width()) / image_.width(),
                               static_cast<double>(height()) / image_.height());
}

QTransform ImageViewer::widgetToLogicalTransform() const {
  return QTransform::fromScale(static_cast<double>(image_.width()) / width(),
                               static_cast<double>(image_.height()) / height());
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
