#pragma once

#include <absl/time/time.h>

#include <cstdint>
#include <span>
#include <vector>

#include "diy/buffer.h"
#include "diy/coro/async_generator.h"

std::span<const std::int16_t> SimulatedSamples();

enum class SimulatedSourcePacing {
  kInstant,
  kRealTime,
};

AsyncGenerator<Buffer<std::int16_t>> SimulatedSource(
    absl::Duration period,
    SimulatedSourcePacing pacing = SimulatedSourcePacing::kInstant);

// A sinusoid whose frequency sweeps between [frequency_min, frequency_max) over
// an interval of `ramp_period`.
struct RampSourceOptions {
  double sample_rate = 24'000;
  absl::Duration frame_period = absl::Milliseconds(10);
  absl::Duration ramp_period = absl::Seconds(1);
  double frequency_min = 1'000;
  double frequency_max = 10'000;
  SimulatedSourcePacing pacing = SimulatedSourcePacing::kInstant;
};

AsyncGenerator<Buffer<std::int16_t>> RampSource(RampSourceOptions options);
