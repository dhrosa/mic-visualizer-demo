#pragma once

#include <cstdint>
#include <vector>

#include "buffer.h"
#include "generator.h"

std::vector<double> FrequencyBins(std::size_t n, double fs);

Generator<Buffer<double>> PowerSpectrum(double sample_rate,
                                        std::size_t window_size,
                                        Generator<Buffer<std::int16_t>> source);
