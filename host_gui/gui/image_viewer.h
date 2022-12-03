#include <QImage>
#include <QSize>
#include <QTransform>
#include <QWidget>
#include <mutex>

class ImageViewer : public QWidget {
 public:
  ImageViewer(std::size_t width, std::size_t height);

  QSize sizeHint() const override { return maximumSize(); }

  QTransform logicalToWidgetTransform() const;
  QTransform widgetToLogicalTransform() const;

  template <typename F>
  void UpdateImage(F&& f);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
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
