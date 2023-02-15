#pragma once

#include <absl/time/time.h>

#include <QImage>
#include <QSize>
#include <cstdint>
#include <vector>

#include "colormaps.h"
#include "diy/buffer.h"
#include "diy/coro/async_generator.h"
#include "diy/rational.h"
#include "image/circular_buffer.h"

class Model {
 public:
  struct Options {
    double sample_rate = 24'000;
    std::size_t fft_window_size = 2028;
    Rational refresh_rate = {60, 1};
  };

  Model();
  Model(const Options& options);

  AsyncGenerator<QImage> Run();

  double FrequencyBin(std::size_t i) const { return frequency_bins_.at(i); }
  std::span<const double> FrequencyBins() const { return frequency_bins_; }

  absl::Duration TimeDelta(std::int64_t n) const;

  QSize imageSize() const noexcept { return QSize(width_, height_); }

 private:
  void AppendSpectrum(Buffer<double> spectrum);

  QImage Render();

  const double sample_rate_;
  const std::size_t fft_window_size_;
  const Rational refresh_rate_;
  const std::vector<double> frequency_bins_;
  const std::size_t width_;
  const std::size_t height_;

  // Audio data in log(psd) form.
  CircularBuffer<double> spectrum_data_;
  // Same data as above, but bucketed into [0, 255] values based on the global
  // min and max observed spectrum values.
  CircularBuffer<std::uint8_t> indexed_data_;
  double min_value_;
  double max_value_;
  const ColorMap* active_colormap_ = &colormaps()[0];
};
