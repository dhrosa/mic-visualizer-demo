#pragma once

#include <complex>
#include <cstdint>
#include <span>
#include <vector>

#include "generator.h"

std::vector<double> PowerSpectrum(std::span<const std::int16_t> samples);

std::vector<double> FrequencyBins(std::size_t n, double fs);
