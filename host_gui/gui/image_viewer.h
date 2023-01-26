#pragma once

#include <absl/synchronization/mutex.h>

#include <QImage>
#include <QSize>
#include <QTransform>
#include <QWidget>

#include "cursor.h"

// Double-buffered widget for rendering a QImage.
class ImageViewer : public QWidget {
  Q_OBJECT
 public:
  ImageViewer(QSize image_size);

  QSize sizeHint() const override { return image_size_; }

  QTransform logicalToWidgetTransform() const;
  QTransform widgetToLogicalTransform() const;

  struct ScopedUpdate {
    ImageViewer& viewer;
    // Image to draw into. This will already have the correct size and format,
    // but unspecified contents.
    QImage& image;

    ~ScopedUpdate() { viewer.EndUpdateImage(); }
  };

  // RAII handle to the double-buffer's secondary image. Once the return value
  // is destructed, this double-buffer is rotated and the newly rendered image
  // will be painted to the widget.
  ScopedUpdate UpdateImage();

 signals:
  void binHovered(QPoint);

 protected:
  void moveEvent(QMoveEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

 private:
  void EndUpdateImage();

  const QSize image_size_;
  Cursor* const cursor_;
  // We double-buffer the images so that the caller can write to one image while
  // we're rendering the previous one without competing for the mutex.
  absl::Mutex mutex_;
  QImage primary_ ABSL_GUARDED_BY(mutex_);
  QImage secondary_ ABSL_GUARDED_BY(mutex_);
};
