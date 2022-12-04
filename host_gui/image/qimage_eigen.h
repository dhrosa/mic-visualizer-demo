#pragma once

#include <Eigen/Core>
#include <QImage>

auto EigenView(QImage& image) {
  using namespace Eigen;
  using ArrayType = Array<std::uint32_t, Dynamic, Dynamic, RowMajor>;
  using MapType = Map<ArrayType, Unaligned, OuterStride<>>;
  const std::size_t rows = image.height();
  const std::size_t cols = image.width();
  return MapType(reinterpret_cast<std::uint32_t*>(image.bits()), rows, cols,
                 OuterStride<>(image.bytesPerLine() / sizeof(std::uint32_t)));
}
