#include "source.h"

#include <xtensor/xadapt.hpp>

// Symbols to access binary data embedded via linker.
extern const std::int16_t _binary_cardinal_pcm_start[];
extern const std::int16_t _binary_cardinal_pcm_end[];

std::span<const std::int16_t> SimulatedSamples() {
  return std::span<const std::int16_t>(_binary_cardinal_pcm_start,
                                       _binary_cardinal_pcm_end);
}
