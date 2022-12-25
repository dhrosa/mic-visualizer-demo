#pragma once

#include <absl/time/time.h>

#include <cstdint>
#include <span>
#include <vector>

#include "diy/buffer.h"
#include "diy/coro/generator.h"

std::span<const std::int16_t> SimulatedSamples();

enum class SimulatedSourcePacing {
  kInstant,
  kRealTime,
};

Generator<Buffer<std::int16_t>> SimulatedSource(
    absl::Duration period,
    SimulatedSourcePacing pacing = SimulatedSourcePacing::kRealTime);
