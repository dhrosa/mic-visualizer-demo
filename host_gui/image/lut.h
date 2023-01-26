#pragma once

#include <Eigen/Core>
#include <cassert>
#include <span>

// Given the input `values` and an equal-length output `indexed`, maps each
// value from the range [min, max] to [0, 255]. Out-of-range values are clamped.
inline void ToIndexed(std::span<const double> values,
                      std::span<std::uint8_t> indexed, double min, double max) {
  assert(values.size() == indexed.size());
  using namespace Eigen;
  auto source =
      Map<const Array<double, Dynamic, 1>>(values.data(), values.size());
  auto dest =
      Map<Array<std::uint8_t, Dynamic, 1>>(indexed.data(), indexed.size());

  // Scales from range [0, (max-min)] to range [0, 255]
  const double scale_factor = 255.0 / (max - min);

  // Clamp input to range [min, max].
  auto clamped = source.max(min).min(max);
  // Shift input to be zero-based.
  auto shifted = clamped - min;
  // Scale to index range.
  auto scaled = shifted * scale_factor;
  // Round and cast to integer.
  dest = (scaled + 0.5).template cast<std::uint8_t>();
}

// Given a 2D array of uint8_t `source`, and a matching dimension 2D array of
// uint32_t `dest`, maps each input value to an output value using the given
// lookup table.
template <typename Source, typename Dest>
void LutMap(Source&& source, Dest&& dest,
            std::span<const std::uint32_t, 256> lut_entries) {
  using namespace Eigen;
  auto lut = Map<const Array<std::uint32_t, 256, 1>>(lut_entries.data());
  const std::size_t rows = source.rows();
  for (std::size_t r = 0; r < rows; ++r) {
    dest.row(r) = lut(source.row(r));
  }
}
