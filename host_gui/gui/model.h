#pragma once

#include <absl/functional/any_invocable.h>

#include <QImage>
#include <QSize>
#include <cstdint>
#include <vector>

#include "colormaps.h"
#include "diy/generator.h"
#include "image/circular_buffer.h"

class Model {
 public:
  Model();

  Generator<absl::AnyInvocable<void(QImage&) &&>> Run();

  QSize imageSize() const { return QSize(data_.width(), data_.height()); }

 private:
  const double sample_rate_ = 24'000;
  const std::size_t fft_window_size_ = 2048;
  const std::vector<double> frequency_bins_;

  CircularBuffer<double> data_;
  double min_value_;
  double max_value_;
  const ColorMap* active_colormap_ = &colormaps()[0];
};
