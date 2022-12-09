#pragma once

#include <absl/functional/any_invocable.h>
#include <absl/time/time.h>

#include <QImage>
#include <QSize>
#include <cstdint>
#include <vector>

#include "colormaps.h"
#include "diy/generator.h"
#include "image/circular_buffer.h"

class Model {
 public:
  Model(double sample_rate = 24'000, std::size_t fft_window_size = 2048);

  Generator<absl::AnyInvocable<void(QImage&) &&>> Run();

  double FrequencyBin(std::size_t i) const { return frequency_bins_.at(i); }

  absl::Duration TimeDelta(std::int64_t n) const;

  QSize imageSize() const noexcept {
    return QSize(data_.width(), data_.height());
  }

 private:
  const double sample_rate_;
  const std::size_t fft_window_size_;
  const std::vector<double> frequency_bins_;

  CircularBuffer<double> data_;
  double min_value_;
  double max_value_;
  const ColorMap* active_colormap_ = &colormaps()[0];
};
