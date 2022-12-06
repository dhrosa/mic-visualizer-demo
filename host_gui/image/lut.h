#pragma once

#include <Eigen/Core>
#include <iostream>
#include <span>

template <typename Scalar>
void LutMapRow(auto&& source, auto&& dest, auto&& lut, Scalar min, Scalar max);

template <typename Scalar, typename Source, typename Dest>
void LutMap(Source&& source, Dest&& dest,
            std::span<const std::uint32_t, 256> lut_entries, Scalar min,
            Scalar max) {
  using namespace Eigen;
  auto lut = Map<const Array<std::uint32_t, 256, 1>>(lut_entries.data());
  const std::size_t rows = source.rows();
#pragma omp parallel for
  for (std::size_t row = 0; row < rows; ++row) {
    LutMapRow(source.row(row), dest.row(row), lut, min, max);
  }
}

template <typename Scalar>
void LutMapRow(auto&& source, auto&& dest, auto&& lut, Scalar min, Scalar max) {
  // Scales from range [0, (max-min)] to range [0, 255]
  const Scalar scale_factor = 255.0 / (max - min);

  // Clamp input to range [min, max].
  auto clamped = source.max(min).min(max);
  // Shift input to be zero-based.
  auto shifted = clamped - min;
  // Scale to index range.
  auto scaled = shifted * scale_factor;
  // Round and cast to integer.
  auto indexes = (scaled + 0.5).template cast<std::uint8_t>();

  dest = lut(indexes);
}
