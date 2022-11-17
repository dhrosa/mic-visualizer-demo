#include "colormaps.h"

#include "colormaps.gen.cc"

std::span<const ColorMap> colormaps() {
  return {kColorMaps};
}
