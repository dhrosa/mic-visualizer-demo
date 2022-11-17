#include <QWidget>
#include <QImage>
#include <QTransform>

#include <mutex>

class ImageViewer : public QWidget {
public:
  ImageViewer();


  QTransform logicalToWidgetTransform() const;
  QTransform widgetToLogicalTransform() const;

  protected:
  void paintEvent(QPaintEvent* event) override;
  
private:
  std::mutex image_mutex_;
  QImage image_;
};
