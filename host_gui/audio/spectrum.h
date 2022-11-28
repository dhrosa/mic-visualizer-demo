#pragma once

#include <complex>
#include <cstdint>
#include <span>
#include <vector>

std::vector<std::complex<double>> Spectrum(
    std::span<const std::int16_t> samples);
