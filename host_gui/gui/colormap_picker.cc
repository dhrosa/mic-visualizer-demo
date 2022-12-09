#include "colormap_picker.h"

#include <memory>

#include "colormaps.h"

namespace {
template <int width, int height>
QIcon GenerateIcon(std::span<const std::uint32_t> lut) {
  // libc++ doesn't support make_unique_for_overwrite as of 20222/12/8
  auto buffer =
      std::unique_ptr<std::uint32_t[]>(new std::uint32_t[width * height * 4]);
  for (int r = 0; r < height; ++r) {
    std::uint32_t* row = &buffer[r * width];
    for (int c = 0; c < width; ++c) {
      row[c] = lut[(r * width + c) % lut.size()];
    }
  }
  const auto* const data = reinterpret_cast<const uchar*>(buffer.get());
  const auto cleanup = [](void* p) { delete[] static_cast<std::uint32_t*>(p); };
  return QIcon(QPixmap::fromImage(QImage(
      data, width, height, QImage::Format_ARGB32, +cleanup, buffer.release())));
}
}  // namespace

ColormapPicker::ColormapPicker() {
  constexpr int width = 256;
  constexpr int height = 24;
  setIconSize(QSize(width, height));
  for (const ColorMap& colormap : colormaps()) {
    addItem(GenerateIcon<width, height>(colormap.entries),
            QString::fromStdString(std::string(colormap.name)));
  }
  previous_text_ = currentText();
}

void ColormapPicker::showPopup() {
  previous_text_ = currentText();
  QComboBox::showPopup();
}

void ColormapPicker::hidePopup() {
  QComboBox::hidePopup();
  setCurrentText(previous_text_);
}
