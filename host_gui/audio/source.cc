#include "source.h"

#include <absl/time/time.h>

#include <algorithm>
#include <memory>
#include <ranges>

#include "diy/coro/sleep.h"

// Symbols to access binary data embedded via linker.
extern const std::int16_t _binary_cardinal_pcm_start[];
extern const std::int16_t _binary_cardinal_pcm_end[];
constexpr std::int64_t kSampleRate = 24'000;

std::span<const std::int16_t> SimulatedSamples() {
  return std::span<const std::int16_t>(_binary_cardinal_pcm_start,
                                       _binary_cardinal_pcm_end);
}

namespace {

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
}  // namespace

AsyncGenerator<Buffer<std::int16_t>> SimulatedSource(
    absl::Duration period, SimulatedSourcePacing pacing) {
  const auto samples = SimulatedSamples();
  const std::size_t frame_size = absl::ToInt64Seconds(kSampleRate * period);
  if (frame_size > samples.size()) {
    throw std::invalid_argument("Period longer than simulated sample source.");
  }
  const absl::Time epoch = absl::Now();
  for (std::size_t frame_num = 0;; ++frame_num) {
    if (pacing == SimulatedSourcePacing::kRealTime) {
      const absl::Time next_frame_time = epoch + frame_num * period;
      co_await Sleep(next_frame_time - absl::Now());
    }
    co_yield PeriodicSubspan(samples, frame_num * frame_size, frame_size);
  }
};
