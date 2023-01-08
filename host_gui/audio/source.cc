#include "source.h"

#include <absl/time/time.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <numbers>
#include <ranges>

#include "diy/coro/executor.h"

// Symbols to access binary data embedded via linker.
extern const std::int16_t _binary_cardinal_pcm_start[];
extern const std::int16_t _binary_cardinal_pcm_end[];
constexpr std::int64_t kSampleRate = 24'000;

std::span<const std::int16_t> SimulatedSamples() {
  return std::span<const std::int16_t>(_binary_cardinal_pcm_start,
                                       _binary_cardinal_pcm_end);
}

namespace {

Buffer<std::int16_t> RampSamples(const RampSourceOptions& options) {
  const std::size_t size =
      absl::ToInt64Seconds(options.sample_rate * options.ramp_period);
  const float f_min = options.frequency_min;
  const float f_max = options.frequency_max;
  auto samples = Buffer<std::int16_t>::Uninitialized(size);
  auto t = std::ranges::iota_view<int, int>(0, size - 1);
  auto y = t | std::views::transform([&](int t) -> std::int16_t {
             const float f =
                 std::lerp(f_min, f_max, static_cast<float>(t) / size);
             const float val = std::sin(2 * std::numbers::pi_v<float> * f *
                                        (t / options.sample_rate));
             return static_cast<std::int16_t>(
                 std::numeric_limits<std::int16_t>::max() * val);
           });
  std::ranges::copy(y, samples.begin());
  return samples;
}

Buffer<std::int16_t> PeriodicSubspan(std::span<const std::int16_t> s,
                                     std::size_t start, std::size_t count) {
  start %= s.size();
  const std::size_t end = std::min(start + count, s.size());
  const std::span first = s.subspan(start, end - start);

  const std::size_t remaining = count - first.size();
  const std::span second = s.subspan(0, remaining);

  auto data = Buffer<std::int16_t>::Uninitialized(count);
  std::ranges::copy(first, data.begin());
  std::ranges::copy(second, data.begin() + first.size());
  return data;
}

AsyncGenerator<Buffer<std::int16_t>> PaceSamples(
    std::span<const std::int16_t> samples, double sample_rate,
    absl::Duration period, SimulatedSourcePacing pacing) {
  const std::size_t frame_size = absl::ToInt64Seconds(sample_rate * period);
  if (frame_size > samples.size()) {
    throw std::invalid_argument("Period longer than simulated sample source.");
  }
  SerialExecutor executor;
  const absl::Time epoch = absl::Now();
  for (std::size_t frame_num = 0;; ++frame_num) {
    if (pacing == SimulatedSourcePacing::kRealTime) {
      const absl::Time next_frame_time = epoch + frame_num * period;
      co_await executor.Sleep(next_frame_time);
    }
    co_yield PeriodicSubspan(samples, frame_num * frame_size, frame_size);
  }
}
}  // namespace

AsyncGenerator<Buffer<std::int16_t>> SimulatedSource(
    absl::Duration period, SimulatedSourcePacing pacing) {
  const auto samples = SimulatedSamples();
  auto frames = PaceSamples(samples, kSampleRate, period, pacing);
  while (auto* frame = co_await frames) {
    co_yield std::move(*frame);
  }
}

AsyncGenerator<Buffer<std::int16_t>> RampSource(RampSourceOptions options) {
  const auto samples = RampSamples(options);
  auto frames = PaceSamples(samples, options.sample_rate, options.frame_period,
                            options.pacing);
  while (auto* frame = co_await frames) {
    co_yield std::move(*frame);
  }
}
