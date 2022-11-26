#include "source.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ranges>
#include <xtensor/xadapt.hpp>

// gMock doesn't pretty pring std::span properly for some reason
auto ToVector(auto s) { return std::vector(s.begin(), s.end()); }

TEST(SimulatedSource, Basic) {
  const std::span<const std::int16_t> samples = SimulatedSamples();
  EXPECT_THAT(samples, testing::SizeIs(101952));

  auto nonzero = [](auto x) { return x != 0; };

  // First non-zero sample is 317 samples away from the start.
  EXPECT_EQ(317, std::distance(samples.begin(),
                               std::ranges::find_if(samples, nonzero)));
  const auto reversed = samples | std::views::reverse;
  // Last non-zero sample is 122 samples away from the end.
  EXPECT_EQ(122, std::distance(reversed.begin(),
                               std::ranges::find_if(reversed, nonzero)));
}
