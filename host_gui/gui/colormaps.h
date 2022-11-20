#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

struct ColorMap {
  std::string_view name;
  std::array<std::uint32_t, 256> entries;
};

std::span<const ColorMap> colormaps();
