#pragma once

#include <cstdint>
#include <vector>

#include "diy/async_generator.h"
#include "diy/buffer.h"

std::vector<double> FrequencyBins(std::size_t n, double fs);

AsyncGenerator<Buffer<double>> PowerSpectrum(
    double sample_rate, std::size_t window_size,
    AsyncGenerator<Buffer<std::int16_t>> source);
