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
#include "audio/spectrum.h"
#include "colormap_picker.h"
#include "colormaps.h"
#include "diy/buffer.h"
#include "diy/generator.h"
#include "diy/latency_logger.h"
#include "image/circular_buffer.h"
#include "image/lut.h"
#include "image/qimage_eigen.h"
#include "image_viewer.h"

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

  const double sample_rate = 24'000;
  const std::size_t fft_window_size = 2048;
  const std::vector<double> frequency_bins =
      FrequencyBins(fft_window_size, sample_rate);
  CircularBuffer<double> data;
  double min_value;
  double max_value;

  const ColorMap* active_colormap = &colormaps()[0];
  std::thread update_thread;
  absl::Notification stopping;
};

MainWindow::Impl::Impl(MainWindow* window)
    : window(window), data(2000, frequency_bins.size()) {
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
  viewer = new ImageViewer(data.width(), data.height());
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
  viewer->UpdateImage([&](QImage& image) {
    auto lut = active_colormap->entries;
    auto dest = EigenView(image);

    auto newer = data.Newer();
    auto older = data.Older();

    LutMap(newer, dest.rightCols(newer.cols()), lut, min_value, max_value);
    LutMap(older, dest.leftCols(older.cols()), lut, min_value, max_value);
  });
}

void MainWindow::Impl::UpdateLoop() {
  auto spectra = PowerSpectrum(sample_rate, fft_window_size,
                               SimulatedSource(absl::Milliseconds(10)));
  for (auto&& spectrum : std::move(spectra)) {
    LatencyLogger logger("UpdateLoop");
    if (stopping.HasBeenNotified()) {
      return;
    }
    std::ranges::for_each(spectrum, [&](auto& v) {
      v = std::log2(v + 1);
      min_value = std::min(v, min_value);
      max_value = std::max(v, max_value);
    });
    data.AppendColumn(spectrum);
    Render();
  }
}

MainWindow::MainWindow() : impl_(std::make_unique<Impl>(this)) {}
MainWindow::~MainWindow() {}
