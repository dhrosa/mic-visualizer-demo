#include <QImage>
#include <QSize>
#include <QTransform>
#include <QWidget>
#include <mutex>

#include "cursor.h"

class ImageViewer : public QWidget {
  Q_OBJECT
 public:
  ImageViewer(std::size_t width, std::size_t height);

  QSize sizeHint() const override { return maximumSize(); }

  QTransform logicalToWidgetTransform() const;
  QTransform widgetToLogicalTransform() const;

  template <typename F>
  void UpdateImage(F&& f);

 signals:
  void binHovered(QPoint);

 protected:
  void enterEvent(QEnterEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

 private:
  Cursor* const cursor_;
  std::mutex image_mutex_;
  QImage image_;
};

template <typename F>
void ImageViewer::UpdateImage(F&& f) {
  {
    auto lock = std::unique_lock(image_mutex_);
    f(image_);
  }
  update();
}
