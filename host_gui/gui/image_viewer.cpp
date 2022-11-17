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

ImageViewer::ImageViewer() : image_(1920, 1080, QImage::Format_ARGB32) {
  const ColorMap cmap = colormaps()[0];

  for (int r = 0; r < image_.height(); ++r) {
    auto* scan_line = reinterpret_cast<std::uint32_t*>(image_.scanLine(r));
    for (int c = 0; c < image_.width(); ++c) {
      scan_line[c] = cmap.entries[(r + c) % 256];
    }
  }
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
