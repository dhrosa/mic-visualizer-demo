#include "source.h"

#include <algorithm>
#include <memory>
#include <ranges>

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

  auto storage = std::make_unique_for_overwrite<std::int16_t[]>(count);
  std::span<std::int16_t> data(storage.get(), count);

  std::ranges::copy(first, data.begin());
  std::ranges::copy(second, data.begin() + first.size());
  return Buffer<std::int16_t>(data, [storage = std::move(storage)] {});
}
}  // namespace

Generator<Buffer<std::int16_t>> SimulatedSource(absl::Duration period) {
  const auto samples = SimulatedSamples();
  const std::size_t frame_size = absl::ToInt64Seconds(kSampleRate * period);
  if (frame_size > samples.size()) {
    throw std::invalid_argument("Period longer than simulated sample source.");
  }
  for (std::size_t frame_num = 0;; ++frame_num) {
    co_yield PeriodicSubspan(samples, frame_num * frame_size, frame_size);
  }
};
