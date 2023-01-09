#include "source.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "diy/coro/task.h"

using testing::ElementsAreArray;
using testing::Pointee;
using testing::SizeIs;

TEST(SimulatedSourceTest, Basic) {
  const std::span<const std::int16_t> samples = SimulatedSamples();
  EXPECT_THAT(samples, SizeIs(101952));

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
  EXPECT_THAT(Task(source).Wait(),
              Pointee(ElementsAreArray(samples.subspan(0, 24))));
  EXPECT_THAT(Task(source).Wait(),
              Pointee(ElementsAreArray(samples.subspan(24, 24))));
  EXPECT_THAT(Task(source).Wait(),
              Pointee(ElementsAreArray(samples.subspan(48, 24))));
  EXPECT_THAT(Task(source).Wait(),
              Pointee(ElementsAreArray(samples.subspan(72, 24))));
}

TEST(SimulatedSourceTest, SourceLastSamplesMatch) {
  const std::span<const std::int16_t> samples = SimulatedSamples();

  auto source = SimulatedSource(absl::Seconds(4));

  EXPECT_THAT(Task(source).Wait(), Pointee(SizeIs(96'000)));

  auto* last = Task(source).Wait();
  ASSERT_NE(last, nullptr);
  EXPECT_THAT(*last, SizeIs(96'000));
  EXPECT_THAT(last->span().first(5952), ElementsAreArray(samples.last(5952)));
  EXPECT_THAT(last->span().last(90048), ElementsAreArray(samples.first(90048)));
}

std::vector<std::int16_t> NextFrame(
    AsyncGenerator<Buffer<std::int16_t>>& source) {
  if (auto* buffer = Task(source).Wait()) {
    return std::vector(buffer->begin(), buffer->end());
  }
  return {};
}

TEST(RampSourceTest, Loops) {
  // This should result in 25 samples per frame in a 4 frame loop. The 5th frame
  // should be identical to the first frame.
  auto source = RampSource({.sample_rate = 100,
                            .frame_period = absl::Milliseconds(250),
                            .ramp_period = absl::Seconds(1),
                            .frequency_min = 0,
                            .frequency_max = 5});
  const std::vector<std::int16_t> frames[] = {
      NextFrame(source), NextFrame(source), NextFrame(source),
      NextFrame(source), NextFrame(source),
  };

  using testing::Eq;
  using testing::Ne;
  EXPECT_THAT(frames[0], SizeIs(25));
  EXPECT_THAT(frames[1], Ne(frames[0]));
  EXPECT_THAT(frames[2], Ne(frames[1]));
  EXPECT_THAT(frames[3], Ne(frames[2]));
  EXPECT_THAT(frames[4], Eq(frames[0]));
}
