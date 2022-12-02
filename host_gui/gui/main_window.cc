#include "main_window.h"

#include <absl/log/log.h>
#include <absl/synchronization/notification.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>

#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "audio/source.h"
#include "colormap_picker.h"
#include "colormaps.h"
#include "diy/buffer.h"
#include "diy/generator.h"
#include "image_viewer.h"

namespace {
void MapColors(const ColorMap& cmap, QImage& image) {
  const int h = image.height();
  const int w = image.width();
  for (int r = 0; r < h; ++r) {
    auto* row = reinterpret_cast<std::uint32_t*>(image.scanLine(r));
    for (int c = 0; c < w; ++c) {
      row[c] = cmap.entries[(r * w + c) % 256];
    }
  }
}
}  // namespace

struct MainWindow::Impl {
  Impl(MainWindow* window);
  ~Impl();

  void initViewer();
  void initToolBar();
  void initStatusBar();
  void initShortcuts();
  void initUpdateThread();

  void SetColormap(int index);
  void Render();
  void UpdateLoop();

  MainWindow* window;
  ImageViewer* viewer;
  const ColorMap* active_colormap = &colormaps()[0];
  std::thread update_thread;
  absl::Notification stopping;
};

MainWindow::Impl::Impl(MainWindow* window) : window(window) {
  initViewer();
  initToolBar();
  initStatusBar();
  initShortcuts();
  initUpdateThread();
}

MainWindow::Impl::~Impl() {
  stopping.Notify();
  update_thread.join();
}

void MainWindow::Impl::initViewer() {
  viewer = new ImageViewer();
  window->setCentralWidget(viewer);
}

void MainWindow::Impl::initToolBar() {
  QToolBar& tool_bar = *window->addToolBar("Tool Bar");
  tool_bar.setFloatable(false);

  auto* colormap_picker = new ColormapPicker();
  tool_bar.addWidget(colormap_picker);

  auto set_colormap = [this](auto&& x) { SetColormap(x); };

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
  frequency_label->setText("1234 Hz");

  status_bar->addPermanentWidget(frequency_label);
}

void MainWindow::Impl::initShortcuts() {
  new QShortcut(QKeySequence::Close, window, [&] { window->close(); });
  new QShortcut(QKeySequence::Quit, window, [&] { window->close(); });
}

void MainWindow::Impl::initUpdateThread() {
  update_thread = std::thread([this] { UpdateLoop(); });
}

void MainWindow::Impl::SetColormap(int index) {
  active_colormap = &colormaps()[index];
  Render();
}

void MainWindow::Impl::Render() {
  viewer->UpdateImage(
      [&](QImage& image) { MapColors(*active_colormap, image); });
}

void MainWindow::Impl::UpdateLoop() {
  while (!stopping.HasBeenNotified()) {
    absl::SleepFor(absl::Seconds(1));
    LOG(INFO) << "sup world";
  }
}

MainWindow::MainWindow() : impl_(std::make_unique<Impl>(this)) {}
MainWindow::~MainWindow() {}
