#include <QMainWindow>
#include <memory>

namespace main_window_internal {
struct Impl;
}  // namespace main_window_internal

class MainWindow : public QMainWindow {
 public:
  MainWindow();
  ~MainWindow();

 private:
  std::unique_ptr<main_window_internal::Impl> impl_;
};
