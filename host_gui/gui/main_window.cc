#include "main_window.h"

#include <absl/log/log.h>
#include <absl/synchronization/mutex.h>

#include <QLabel>
#include <QShortcut>
#include <QStatusBar>
#include <QToolBar>
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

  void UpdateLoop();

  Model model;

  absl::Mutex mutex;
  bool stopping ABSL_GUARDED_BY(mutex) = false;

  MainWindow* window;
  ImageViewer* viewer;

  std::vector<std::thread> threads;
};

MainWindow::Impl::Impl(MainWindow* window) : window(window) {
  initViewer();
  initToolBar();
  initStatusBar();
  initShortcuts();

  threads.emplace_back([this] { UpdateLoop(); });
}

MainWindow::Impl::~Impl() {
  {
    absl::MutexLock lock(&mutex);
    stopping = true;
  }
  for (std::thread& thread : threads) {
    thread.join();
  }
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
  frequency_label->setText("1234 Hz");

  status_bar->addPermanentWidget(frequency_label);
}

void MainWindow::Impl::initShortcuts() {
  new QShortcut(QKeySequence::Close, window, [&] { window->close(); });
  new QShortcut(QKeySequence::Quit, window, [&] { window->close(); });
}

void MainWindow::Impl::UpdateLoop() {
  for (auto&& render : model.Run()) {
    {
      absl::MutexLock lock(&mutex);
      if (stopping) {
        return;
      }
    }
    viewer->UpdateImage(std::move(render));
  }
}

MainWindow::MainWindow() : impl_(std::make_unique<Impl>(this)) {}
MainWindow::~MainWindow() {}
