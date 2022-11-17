#include "main_window.h"

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "image_viewer.h"

namespace main_window_internal {
struct Impl {
  Impl(MainWindow* window);

  void initViewer();
  void initStatusBar();
  void initShortcuts();

  MainWindow* window;
  ImageViewer* viewer;
};

Impl::Impl(MainWindow* window) : window(window) {
  initViewer();
  initStatusBar();
  initShortcuts();
}

void Impl::initViewer() {
  auto scoped_viewer = std::make_unique<ImageViewer>();
  viewer = scoped_viewer.get();
  window->setCentralWidget(std::move(scoped_viewer).release());
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

}  // namespace main_window_internal

MainWindow::MainWindow()
    : impl_(std::make_unique<main_window_internal::Impl>(this)) {}
MainWindow::~MainWindow() {}
