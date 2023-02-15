#include "main_window.h"

#include <absl/log/log.h>
#include <absl/strings/str_format.h>
#include <absl/time/time.h>

#include <QLabel>
#include <QShortcut>
#include <QStatusBar>
#include <QToolBar>
#include <stop_token>
#include <thread>

#include "colormap_picker.h"
#include "diy/coro/task.h"
#include "image_viewer.h"
#include "model.h"
#include "scroll_area.h"

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
  ScrollArea* scroll_area;

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
  scroll_area = new ScrollArea();
  scroll_area->setWidget(viewer);
  window->setCentralWidget(scroll_area);
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
  status_bar->addPermanentWidget(frequency_label);

  auto* time_label = new QLabel();
  time_label->setFont(font);
  status_bar->addPermanentWidget(time_label);

  QObject::connect(viewer, &ImageViewer::binHovered, [=, this](QPoint p) {
    const std::span<const double> bins = model.FrequencyBins();
    // Flip Y-axis from graphical convention to math convention.
    const double f = *(bins.rbegin() + p.y());
    frequency_label->setText(
        QString::fromStdString(absl::StrFormat("%.2f Hz", f)));

    const int t_index = p.x() - model.imageSize().width() - 1;
    const absl::Duration t =
        absl::Floor(model.TimeDelta(t_index), absl::Milliseconds(1));
    time_label->setText(QString::fromStdString(
        absl::StrFormat("T=%s", absl::FormatDuration(t))));
  });
}

void MainWindow::Impl::initShortcuts() {
  new QShortcut(QKeySequence::Close, window, [&] { window->close(); });
  new QShortcut(QKeySequence::Quit, window, [&] { window->close(); });
}

void MainWindow::Impl::UpdateLoop(std::stop_token stop_token) {
  [this](std::stop_token stop_token) -> Task<> {
    auto frames = model.Run();
    while (QImage* frame = co_await frames) {
      if (stop_token.stop_requested()) {
        co_return;
      }
      ImageViewer::ScopedUpdate update = viewer->UpdateImage();
      update.image = std::move(*frame);
    }
  }(stop_token)
                                            .Wait();
}

MainWindow::MainWindow() : impl_(std::make_unique<Impl>(this)) {}
MainWindow::~MainWindow() {}
