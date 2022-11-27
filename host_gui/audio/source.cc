#include "source.h"

#include <algorithm>
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

std::vector<std::int16_t> PeriodicSubspan(std::span<const std::int16_t> s,
                                          std::size_t start,
                                          std::size_t count) {
  start %= s.size();
  const std::size_t end = std::min(start + count, s.size());
  const std::span first = s.subspan(start, end - start);

  const std::size_t remaining = count - first.size();
  const std::span second = s.subspan(0, remaining);

  std::vector<std::int16_t> out;
  out.reserve(count);
  std::ranges::copy(first, std::back_insert_iterator(out));
  std::ranges::copy(second, std::back_insert_iterator(out));
  return out;
}
}  // namespace

Generator<std::vector<std::int16_t>> SimulatedSource(absl::Duration period) {
  const auto samples = SimulatedSamples();
  const std::size_t frame_size = absl::ToInt64Seconds(kSampleRate * period);
  if (frame_size > samples.size()) {
    throw std::invalid_argument("Period longer than simulated sample source.");
  }
  for (std::size_t frame_num = 0;; ++frame_num) {
    co_yield PeriodicSubspan(samples, frame_num * frame_size, frame_size);
  }
};
