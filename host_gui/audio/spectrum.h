#pragma once

#include <cstdint>
#include <vector>

#include "diy/buffer.h"
#include "diy/coro/async_generator.h"

std::vector<double> FrequencyBins(std::size_t n, double fs);

enum class WindowFunction {
  kRectangular,
  kHann,
};

struct SpectrumOptions {
  double sample_rate = 24'000;
  std::size_t window_size = 2048;
  WindowFunction window_function = WindowFunction::kRectangular;
};

AsyncGenerator<Buffer<double>> PowerSpectrum(
    SpectrumOptions options, AsyncGenerator<Buffer<std::int16_t>> source);
