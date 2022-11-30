#pragma once

#include <complex>
#include <cstdint>
#include <span>
#include <vector>

#include "buffer.h"
#include "generator.h"

Buffer<double> PowerSpectrum(std::span<const std::int16_t> samples,
                             double fs = 24'000);

std::vector<double> FrequencyBins(std::size_t n, double fs);
