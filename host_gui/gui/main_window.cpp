#include "main_window.h"

#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "colormap_picker.h"
#include "colormaps.h"
#include "image_viewer.h"
#include "generator.h"

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

namespace main_window_internal {
struct Impl {
  Impl(MainWindow* window);

  void initViewer();
  void initToolBar();
  void initStatusBar();
  void initShortcuts();

  void SetColormap(int index);
  void Render();

  MainWindow* window;
  ImageViewer* viewer;
  const ColorMap* active_colormap = &colormaps()[0];
};

Impl::Impl(MainWindow* window) : window(window) {
  initViewer();
  initToolBar();
  initStatusBar();
  initShortcuts();
}

void Impl::initViewer() {
  auto scoped_viewer = std::make_unique<ImageViewer>();
  viewer = scoped_viewer.get();
  window->setCentralWidget(std::move(scoped_viewer).release());
}

void Impl::initToolBar() {
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

void Impl::initStatusBar() {
  QStatusBar* status_bar = window->statusBar();

  auto* frequency_label = new QLabel();
  QFont font = frequency_label->font();
  font.setFamily("monospace");
  frequency_label->setFont(font);
  frequency_label->setText("1234 Hz");

  status_bar->addPermanentWidget(frequency_label);
}

void Impl::initShortcuts() {
  new QShortcut(QKeySequence::Close, window, [&] { window->close(); });
  new QShortcut(QKeySequence::Quit, window, [&] { window->close(); });
}

void Impl::SetColormap(int index) {
  active_colormap = &colormaps()[index];
  Render();
}

void Impl::Render() {
  viewer->UpdateImage(
      [&](QImage& image) { MapColors(*active_colormap, image); });
}

}  // namespace main_window_internal

MainWindow::MainWindow()
    : impl_(std::make_unique<main_window_internal::Impl>(this)) {}
MainWindow::~MainWindow() {}
