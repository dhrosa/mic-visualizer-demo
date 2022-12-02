#include <QMainWindow>
#include <memory>

class MainWindow : public QMainWindow {
 public:
  MainWindow();
  ~MainWindow();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
