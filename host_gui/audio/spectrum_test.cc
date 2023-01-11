#include "spectrum.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ranges>

#include "diy/coro/task.h"

using testing::ElementsAre;
using testing::IsNull;
using testing::Pointee;
using testing::SizeIs;

template <typename T>
void PrintTo(const Buffer<T>& buffer, std::ostream* s) {
  *s << testing::PrintToString(std::vector<T>(buffer.begin(), buffer.end()));
}

AsyncGenerator<Buffer<std::int16_t>> Source(
    std::vector<std::vector<std::int16_t>> frames) {
  for (const auto& frame : frames) {
    auto buffer = Buffer<std::int16_t>::Uninitialized(frame.size());
    std::ranges::copy(frame, buffer.begin());
    co_yield std::move(buffer);
  }
}

AsyncGenerator<Buffer<std::int16_t>> SingleFrameSource(
    std::vector<std::int16_t> head) {
  return Source({head});
}

TEST(SpectrumTest, OddSizeThrowsError) {
  auto gen = PowerSpectrum(2, 3, SingleFrameSource({0}));
  EXPECT_THROW(Task(gen).Wait(), std::invalid_argument);
}

TEST(SpectrumTest, Zero) {
  auto gen = PowerSpectrum(2, 4, SingleFrameSource({0, 0, 0, 0}));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(0, 0, 0)));
  EXPECT_THAT(Task(gen).Wait(), IsNull());
}

TEST(SpectrumTest, Dc) {
  auto gen = PowerSpectrum(2, 4, SingleFrameSource({1, 1, 1, 1}));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(1, 0, 0)));
  EXPECT_THAT(Task(gen).Wait(), IsNull());
}

TEST(SpectrumTest, NyquistRate) {
  auto gen = PowerSpectrum(2, 4, SingleFrameSource({1, -1, 1, -1}));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(0, 0, 1)));
  EXPECT_THAT(Task(gen).Wait(), IsNull());
}

TEST(SpectrumTest, DcAndNyquist) {
  auto gen = PowerSpectrum(2, 4, SingleFrameSource({2, 0, 2, 0}));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(1, 0, 1)));
  EXPECT_THAT(Task(gen).Wait(), IsNull());
}

TEST(SpectrumTest, Cosine) {
  auto gen = PowerSpectrum(2, 4, SingleFrameSource({1, 0, -1, 0}));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(0, 0.5, 0)));
  EXPECT_THAT(Task(gen).Wait(), IsNull());
}

TEST(SpectrumTest, Sine) {
  auto gen = PowerSpectrum(2, 4, SingleFrameSource({0, 1, 0, -1}));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(0, 0.5, 0)));
  EXPECT_THAT(Task(gen).Wait(), IsNull());
}

TEST(SpectrumTest, MergesFrames) {
  auto gen = PowerSpectrum(
      2, 4,
      Source({{0, 0, 0}, {0, 1}, {1, 1}, {1, 0, 1, 0, -1, 1, -1, 1, -1, 1}}));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(0, 0, 0)));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(1, 0, 0)));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(0, 0.5, 0)));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(0, 0, 1)));
  EXPECT_THAT(Task(gen).Wait(), IsNull());
}

TEST(SpectrumTest, EmptyInput) {
  auto gen = PowerSpectrum(2, 4, SingleFrameSource({}));
  EXPECT_THAT(Task(gen).Wait(), IsNull());
}

TEST(SpectrumTest, CleanlyTruncatedInput) {
  auto gen = PowerSpectrum(2, 4, SingleFrameSource({1, 1, 1, 1}));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(1, 0, 0)));
  EXPECT_THAT(Task(gen).Wait(), IsNull());
}

TEST(SpectrumTest, TruncatedInput) {
  auto gen = PowerSpectrum(2, 4, SingleFrameSource({1, 1, 1, 1, 0, 0}));
  EXPECT_THAT(Task(gen).Wait(), Pointee(ElementsAre(1, 0, 0)));
  EXPECT_THAT(Task(gen).Wait(), IsNull());
}

TEST(BinsTest, EvenSize) {
  EXPECT_THAT(FrequencyBins(10, 1000), ElementsAre(0, 100, 200, 300, 400, 500));
}

TEST(BinsTest, OddSizeThrowsError) {
  EXPECT_THROW(FrequencyBins(9, 1000), std::invalid_argument);
}
