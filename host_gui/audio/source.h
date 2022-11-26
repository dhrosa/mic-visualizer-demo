#pragma once

#include <cstdint>
#include <span>
#include <xtensor/xtensor.hpp>

#include "generator.h"

using AudioFrame = xt::xtensor<std::int16_t, 1>;

Generator<AudioFrame> SimulatedSource();

std::span<const std::int16_t> SimulatedSamples();
