#pragma once

#include <QtGui/QImage>
#include <cstdlib>
#include <stdexcept>

// AVX2 instructions expect 32-byte alignment.
constexpr unsigned kDefaultQImageAlignment = 32;

// Creates a QImage of the given dimensions where each scan line is aligned to
// the number of bytes given by `alignment`. `alignment` must be a power of 2
// greater than 4.
inline QImage AlignedQImage(int width, int height,
                            unsigned alignment = kDefaultQImageAlignment,
                            QImage::Format format = QImage::Format_RGB32) {
  if (!std::has_single_bit(alignment)) {
    throw std::invalid_argument("Alignment must be a power of 2.");
  }
  if (alignment < 4) {
    throw std::invalid_argument("Alignment must be at least 4 bytes.");
  }
  const double width_blocks = static_cast<double>(width) / alignment;
  const int pixel_stride = std::ceil(width_blocks) * alignment;
  const int bytes_per_line = pixel_stride * 4;

  auto* data = reinterpret_cast<unsigned char*>(
      std::aligned_alloc(alignment, height * bytes_per_line));
  constexpr auto cleanup = [](void* data) {
    delete reinterpret_cast<unsigned char*>(data);
  };
  return QImage(data, width, height, bytes_per_line, format, cleanup, data);
}
