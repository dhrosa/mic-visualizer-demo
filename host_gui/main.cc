#include <absl/log/globals.h>
#include <absl/log/initialize.h>

#include <QApplication>

#include "gui/main_window.h"

int main(int argc, char** argv) {
  absl::InitializeLog();
  absl::SetStderrThreshold(absl::LogSeverity::kInfo);
  QApplication app(argc, argv);
  MainWindow window;
  window.show();
  return app.exec();
}
