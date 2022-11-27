#pragma once

#include <absl/time/time.h>

#include <cstdint>
#include <span>
#include <vector>

#include "generator.h"

std::span<const std::int16_t> SimulatedSamples();

Generator<std::vector<std::int16_t>> SimulatedSource(absl::Duration period);
