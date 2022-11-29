#pragma once

#include <complex>
#include <cstdint>
#include <span>
#include <vector>

#include "generator.h"

std::vector<std::complex<double>> Spectrum(
    std::span<const std::int16_t> samples);

std::vector<double> PowerSpectrum(std::span<const std::int16_t> samples);
