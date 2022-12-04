#pragma once

#include <Eigen/Core>
#include <iostream>
#include <span>

template <typename Source, typename Dest>
void LutMap(const Source& source, Dest& dest,
            std::span<const std::uint32_t, 256> lut_entries, double min,
            double max) {
  using namespace Eigen;
  auto lut = Map<const Array<std::uint32_t, lut_entries.size(), 1>>(
      lut_entries.data());

  auto clamped = source.max(min).min(max);
  auto shifted = clamped - min;
  auto scaled = (shifted / (max - min)) * 255;
  auto indexes = scaled.template cast<std::uint8_t>().eval();
  for (std::size_t row = 0; row < source.rows(); ++row) {
    dest.row(row) = lut(indexes.row(row));
  }
}
