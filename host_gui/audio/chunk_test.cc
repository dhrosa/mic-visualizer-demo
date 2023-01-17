#include "chunk.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ranges>

#include "diy/coro/task.h"

using testing::ElementsAre;
using testing::IsNull;
using testing::Pointee;

auto* NextValue(auto& gen) { return Task(gen).Wait(); }

template <typename... Args>
testing::Matcher<AsyncGenerator<std::vector<int>>> OutputFramesAre(
    Args&&... matchers) {
  auto output_frames = [](AsyncGenerator<std::vector<int>> generator) {
    auto range = generator.ToSyncRange();
    return std::vector(range.begin(), range.end());
  };
  return testing::ResultOf("output frames", output_frames,
                           ElementsAre(std::forward<Args>(matchers)...));
}

TEST(ChunkTest, Empty) {
  auto chunked =
      Chunked([]() -> AsyncGenerator<std::vector<int>> { co_return; }(), 3);
  EXPECT_THAT(NextValue(chunked), IsNull());
}

TEST(ChunkTest, TruncatedInputIntraFrame) {
  auto source = []() -> AsyncGenerator<std::vector<int>> {
    co_yield {1, 2, 3};
    co_yield {4, 5};
  };
  auto chunked = Chunked(source(), 3);
  EXPECT_THAT(NextValue(chunked), Pointee(ElementsAre(1, 2, 3)));
  EXPECT_THAT(NextValue(chunked), IsNull());
}

TEST(ChunkTest, TruncatedInputInterFrame) {
  auto source = []() -> AsyncGenerator<std::vector<int>> {
    co_yield {1, 2, 3};
  };
  auto chunked = Chunked(source(), 3);
  EXPECT_THAT(NextValue(chunked), Pointee(ElementsAre(1, 2, 3)));
  EXPECT_THAT(NextValue(chunked), IsNull());
}

TEST(ChunkTest, MergesSmallFrames) {
  auto source = []() -> AsyncGenerator<std::vector<int>> {
    co_yield {};
    co_yield {1};
    co_yield {2, 3};
    co_yield {4};
    co_yield {5};
    co_yield {6};
  };
  auto chunked = Chunked(source(), 3);
  EXPECT_THAT(NextValue(chunked), Pointee(ElementsAre(1, 2, 3)));
  EXPECT_THAT(NextValue(chunked), Pointee(ElementsAre(4, 5, 6)));
}

TEST(ChunkTest, SplitsLargeFrames) {
  auto source = []() -> AsyncGenerator<std::vector<int>> {
    co_yield {1, 2, 3, 4, 5, 6};
  };
  auto chunked = Chunked(source(), 3);
  EXPECT_THAT(NextValue(chunked), Pointee(ElementsAre(1, 2, 3)));
  EXPECT_THAT(NextValue(chunked), Pointee(ElementsAre(4, 5, 6)));
}

// This case requires combining data from separate input frames and also using
// a single input frame for multiple output frames.
TEST(ChunkTest, UnalignedFrames) {
  auto source = []() -> AsyncGenerator<std::vector<int>> {
    co_yield {1, 2, 3, 4};
    co_yield {5, 6, 7, 8};
  };
  auto chunked = Chunked(source(), 3);
  EXPECT_THAT(NextValue(chunked), Pointee(ElementsAre(1, 2, 3)));
  EXPECT_THAT(NextValue(chunked), Pointee(ElementsAre(4, 5, 6)));
}
