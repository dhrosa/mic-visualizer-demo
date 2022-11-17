#include "colormap_picker.h"

#include "colormaps.h"

#include <memory>

namespace {
  template <int width, int height>
  QIcon GenerateIcon(std::span<const std::uint32_t> lut) {
    using Buffer = std::array<std::uint32_t, width*height*4>;
    auto buffer = std::make_unique_for_overwrite<Buffer>();
    std::uint32_t* data = buffer->data();
    for (int r = 0; r < height; ++r) {
      std::uint32_t* row = &data[r * width];
      for (int c = 0; c < width; ++c) {
        row[c] = lut[(r * width + c) % lut.size()];
      }
    }
    const auto cleanup = [](void* p) {
      delete static_cast<Buffer*>(p);
    };
    return QIcon(QPixmap::fromImage(QImage(reinterpret_cast<const uchar*>(buffer->data()),
                                           width, height, QImage::Format_ARGB32, +cleanup, buffer.release())));
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
