#pragma once

#include <cstdint>
#include <vector>

#include "diy/buffer.h"
#include "diy/coro/async_generator.h"

std::vector<double> FrequencyBins(std::size_t n, double fs);

AsyncGenerator<Buffer<double>> PowerSpectrum(
    double sample_rate, std::size_t window_size,
    AsyncGenerator<Buffer<std::int16_t>> source);
