#include "main_window.h"

#include <absl/log/log.h>
#include <absl/strings/str_format.h>

#include <QLabel>
#include <QShortcut>
#include <QStatusBar>
#include <QToolBar>
#include <stop_token>
#include <thread>

#include "colormap_picker.h"
#include "image_viewer.h"
#include "model.h"

struct MainWindow::Impl {
  Impl(MainWindow* window);
  ~Impl();

  void initViewer();
  void initToolBar();
  void initStatusBar();
  void initShortcuts();

  void UpdateLoop(std::stop_token stop_token);

  Model model;

  MainWindow* window;
  ImageViewer* viewer;

  std::jthread update_thread;
};

MainWindow::Impl::Impl(MainWindow* window) : window(window) {
  initViewer();
  initToolBar();
  initStatusBar();
  initShortcuts();

  update_thread = std::jthread(&Impl::UpdateLoop, this);
}

MainWindow::Impl::~Impl() {
  update_thread.request_stop();
  update_thread.join();
}

void MainWindow::Impl::initViewer() {
  viewer = new ImageViewer(model.imageSize());
  window->setCentralWidget(viewer);
}

void MainWindow::Impl::initToolBar() {
  QToolBar& tool_bar = *window->addToolBar("Tool Bar");
  tool_bar.setFloatable(false);

  auto* colormap_picker = new ColormapPicker();
  tool_bar.addWidget(colormap_picker);

  auto set_colormap = [this](auto&& x) {
    LOG(INFO) << "Colormap switching not yet available.";
  };

  QObject::connect(colormap_picker, &ColormapPicker::highlighted, window,
                   set_colormap);
  QObject::connect(colormap_picker, &ColormapPicker::currentIndexChanged,
                   window, set_colormap);
}

void MainWindow::Impl::initStatusBar() {
  QStatusBar* status_bar = window->statusBar();
  auto* frequency_label = new QLabel();
  QFont font = frequency_label->font();
  font.setFamily("monospace");
  frequency_label->setFont(font);

  QObject::connect(
      viewer, &ImageViewer::binHovered, [this, frequency_label](QPoint p) {
        frequency_label->setText(QString::fromStdString(
            absl::StrFormat("%.2f Hz", model.FrequencyBin(p.y()))));
      });

  status_bar->addPermanentWidget(frequency_label);
}

void MainWindow::Impl::initShortcuts() {
  new QShortcut(QKeySequence::Close, window, [&] { window->close(); });
  new QShortcut(QKeySequence::Quit, window, [&] { window->close(); });
}

void MainWindow::Impl::UpdateLoop(std::stop_token stop_token) {
  for (auto&& render : model.Run()) {
    if (stop_token.stop_requested()) {
      return;
    }
    viewer->UpdateImage(std::move(render));
  }
}

MainWindow::MainWindow() : impl_(std::make_unique<Impl>(this)) {}
MainWindow::~MainWindow() {}
