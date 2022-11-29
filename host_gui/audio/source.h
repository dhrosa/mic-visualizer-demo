#pragma once

#include <absl/time/time.h>

#include <cstdint>
#include <span>
#include <vector>

#include "generator.h"
#include "buffer.h"

std::span<const std::int16_t> SimulatedSamples();

Generator<Buffer<std::int16_t>> SimulatedSource(absl::Duration period);
