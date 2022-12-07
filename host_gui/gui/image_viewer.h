#include <absl/functional/function_ref.h>
#include <absl/synchronization/mutex.h>

#include <QImage>
#include <QSize>
#include <QTransform>
#include <QWidget>

#include "cursor.h"

class ImageViewer : public QWidget {
  Q_OBJECT
 public:
  ImageViewer(std::size_t width, std::size_t height);

  QSize sizeHint() const override { return maximumSize(); }

  QTransform logicalToWidgetTransform() const;
  QTransform widgetToLogicalTransform() const;

  void UpdateImage(absl::FunctionRef<void(QImage&)> f);

 signals:
  void binHovered(QPoint);

 protected:
  void enterEvent(QEnterEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

 private:
  Cursor* const cursor_;
  absl::Mutex mutex_;
  QImage image_ ABSL_GUARDED_BY(mutex_);
};
