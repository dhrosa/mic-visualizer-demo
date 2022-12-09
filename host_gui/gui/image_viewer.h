#pragma once

#include <absl/functional/any_invocable.h>
#include <absl/synchronization/mutex.h>

#include <QImage>
#include <QSize>
#include <QTransform>
#include <QWidget>

#include "cursor.h"

class ImageViewer : public QWidget {
  Q_OBJECT
 public:
  ImageViewer(QSize image_size);

  QSize sizeHint() const override { return image_size_; }

  QTransform logicalToWidgetTransform() const;
  QTransform widgetToLogicalTransform() const;

  void UpdateImage(absl::AnyInvocable<void(QImage&) &&> f);

 signals:
  void binHovered(QPoint);

 protected:
  void moveEvent(QMoveEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

 private:
  const QSize image_size_;
  Cursor* const cursor_;
  absl::Mutex mutex_;
  QImage image_ ABSL_GUARDED_BY(mutex_);
};
