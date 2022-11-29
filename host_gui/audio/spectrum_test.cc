#include "spectrum.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::ElementsAre;
using testing::SizeIs;

TEST(SpectrumTest, OddSizeThrowsError) {
  const std::int16_t samples[] = {0, 0, 0};
  EXPECT_THROW(PowerSpectrum(samples), std::invalid_argument);
}

TEST(SpectrumTest, Zero) {
  const std::int16_t samples[] = {0, 0, 0, 0};
  EXPECT_THAT(PowerSpectrum(samples), ElementsAre(0, 0, 0));
}

TEST(SpectrumTest, Dc) {
  const std::int16_t samples[] = {1, 1, 1, 1};
  EXPECT_THAT(PowerSpectrum(samples), ElementsAre(4, 0, 0));
}

TEST(SpectrumTest, NyquistRate) {
  const std::int16_t samples[] = {1, -1, 1, -1};
  EXPECT_THAT(PowerSpectrum(samples), ElementsAre(0, 0, 4));
}

TEST(SpectrumTest, DcAndNyquist) {
  const std::int16_t samples[] = {2, 0, 2, 0};
  EXPECT_THAT(PowerSpectrum(samples), ElementsAre(4, 0, 4));
}

TEST(SpectrumTest, Cosine) {
  const std::int16_t samples[] = {1, 0, -1, 0};
  EXPECT_THAT(PowerSpectrum(samples), ElementsAre(0, 1, 0));
}

TEST(SpectrumTest, Sine) {
  const std::int16_t samples[] = {0, 1, 0, -1};
  EXPECT_THAT(PowerSpectrum(samples), ElementsAre(0, 1, 0));
}

TEST(BinsTest, EvenSize) {
  EXPECT_THAT(FrequencyBins(10, 1000), ElementsAre(0, 100, 200, 300, 400, 500));
}

TEST(BinsTest, OddSizeThrowsError) {
  EXPECT_THROW(FrequencyBins(9, 1000), std::invalid_argument);
}
