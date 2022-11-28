#include "source.h"

#include <absl/debugging/failure_signal_handler.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ranges>

using testing::AllOf;
using testing::ElementsAreArray;
using testing::SizeIs;

// gMock doesn't pretty pring std::span properly for some reason
auto ToVector(auto s) { return std::vector(s.begin(), s.end()); }

TEST(SimulatedSourceTest, Basic) {
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

TEST(SimulatedSourceTest, SourceFirstSamplesMatch) {
  const std::span<const std::int16_t> samples = SimulatedSamples();
  // 1ms @ 24kHz = 24 samples.
  auto source = SimulatedSource(absl::Milliseconds(1));
  EXPECT_THAT(source(), ElementsAreArray(samples.subspan(0, 24)));
  EXPECT_THAT(source(), ElementsAreArray(samples.subspan(24, 48)));
  EXPECT_THAT(source(), ElementsAreArray(samples.subspan(48, 72)));
  EXPECT_THAT(source(), ElementsAreArray(samples.subspan(72, 96)));
}

TEST(SimulatedSourceTest, SourceLastSamplesMatch) {
  const std::span<const std::int16_t> samples = SimulatedSamples();

  auto source = SimulatedSource(absl::Seconds(4));

  EXPECT_THAT(source(), SizeIs(96'000));

  auto last = source();
  EXPECT_THAT(last, SizeIs(96'000));
  EXPECT_THAT(std::span<const std::int16_t>(last).first(5952),
              ElementsAreArray(samples.last(5952)));
  EXPECT_THAT(std::span<const std::int16_t>(last).last(90048),
              ElementsAreArray(samples.first(90048)));
}

int main(int argc, char** argv) {
  absl::InstallFailureSignalHandler({});
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
