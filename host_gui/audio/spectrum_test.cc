#include "spectrum.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ranges>

using testing::ElementsAre;
using testing::SizeIs;

template <typename T>
void PrintTo(const Buffer<T>& buffer, std::ostream* s) {
  *s << testing::PrintToString(std::vector<T>(buffer.begin(), buffer.end()));
}

Generator<Buffer<std::int16_t>> Source(
    std::vector<std::vector<std::int16_t>> frames) {
  for (auto frame : frames) {
    auto buffer = Buffer<std::int16_t>::Uninitialized(frame.size());
    std::ranges::copy(frame, buffer.begin());
    co_yield std::move(buffer);
  }
}

Generator<Buffer<std::int16_t>> SingleFrameSource(
    std::vector<std::int16_t> head) {
  return Source({head});
}

TEST(SpectrumTest, OddSizeThrowsError) {
  EXPECT_THROW(PowerSpectrum(2, 3, SingleFrameSource({0}))(),
               std::invalid_argument);
}

TEST(SpectrumTest, Zero) {
  EXPECT_THAT(PowerSpectrum(2, 4, SingleFrameSource({0, 0, 0, 0}))(),
              ElementsAre(0, 0, 0));
}

TEST(SpectrumTest, Dc) {
  EXPECT_THAT(PowerSpectrum(2, 4, SingleFrameSource({1, 1, 1, 1}))(),
              ElementsAre(1, 0, 0));
}

TEST(SpectrumTest, NyquistRate) {
  EXPECT_THAT(PowerSpectrum(2, 4, SingleFrameSource({1, -1, 1, -1}))(),
              ElementsAre(0, 0, 1));
}

TEST(SpectrumTest, DcAndNyquist) {
  EXPECT_THAT(PowerSpectrum(2, 4, SingleFrameSource({2, 0, 2, 0}))(),
              ElementsAre(1, 0, 1));
}

TEST(SpectrumTest, Cosine) {
  EXPECT_THAT(PowerSpectrum(2, 4, SingleFrameSource({1, 0, -1, 0}))(),
              ElementsAre(0, 0.5, 0));
}

TEST(SpectrumTest, Sine) {
  EXPECT_THAT(PowerSpectrum(2, 4, SingleFrameSource({0, 1, 0, -1}))(),
              ElementsAre(0, 0.5, 0));
}

TEST(SpectrumTest, MergesFrames) {
  auto gen = PowerSpectrum(
      2, 4,
      Source({{0, 0, 0}, {0, 1}, {1, 1}, {1, 0, 1, 0, -1, 1, -1, 1, -1, 1}}));
  EXPECT_THAT(gen(), ElementsAre(0, 0, 0));
  EXPECT_THAT(gen(), ElementsAre(1, 0, 0));
  EXPECT_THAT(gen(), ElementsAre(0, 0.5, 0));
  EXPECT_THAT(gen(), ElementsAre(0, 0, 1));
}

TEST(BinsTest, EvenSize) {
  EXPECT_THAT(FrequencyBins(10, 1000), ElementsAre(0, 100, 200, 300, 400, 500));
}

TEST(BinsTest, OddSizeThrowsError) {
  EXPECT_THROW(FrequencyBins(9, 1000), std::invalid_argument);
}
