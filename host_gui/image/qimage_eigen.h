#pragma once

#include <Eigen/Core>
#include <QImage>
#include <concepts>
#include <type_traits>

template <typename T>
concept IsQImageReference =
    std::is_lvalue_reference_v<T> && std::same_as<QImage, std::decay_t<T>>;

// Converts a QImage reference to an Eigen-compatible view of the image data,
// while preserving the const-ness of the input. By presrving const-ness, we
// prevent unintentional copy-on-write behavior from QImage when only const
// access is required.
inline auto EigenView(IsQImageReference auto&& image) {
  constexpr bool is_const =
      std::is_const_v<std::remove_reference_t<decltype(image)>>;
  using ElementType =
      std::conditional_t<is_const, const std::uint32_t, std::uint32_t>;
  ElementType* data;
  if constexpr (is_const) {
    data = reinterpret_cast<ElementType*>(image.constBits());
  } else {
    data = reinterpret_cast<ElementType*>(image.bits());
  }
  using namespace Eigen;
  using ArrayType = Array<std::uint32_t, Dynamic, Dynamic, RowMajor>;
  using MapType = Map<std::conditional_t<is_const, const ArrayType, ArrayType>,
                      Unaligned, OuterStride<>>;
  const std::size_t rows = image.height();
  const std::size_t cols = image.width();
  return MapType(data, rows, cols,
                 OuterStride<>(image.bytesPerLine() / sizeof(std::uint32_t)));
}
