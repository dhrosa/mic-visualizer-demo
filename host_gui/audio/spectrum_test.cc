#include "spectrum.h"

#include <absl/debugging/failure_signal_handler.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std::literals::complex_literals;
using testing::AllOf;
using testing::Each;
using testing::ElementsAre;
using testing::SizeIs;

TEST(SpectrumTest, Zero) {
  const std::int16_t samples[] = {0, 0, 0, 0};
  EXPECT_THAT(Spectrum(samples), AllOf(SizeIs(4), Each(0)));
}

TEST(SpectrumTest, Dc) {
  const std::int16_t samples[] = {1, 1, 1, 1};
  EXPECT_THAT(Spectrum(samples), ElementsAre(4, 0, 0, 0));
}

TEST(SpectrumTest, NyquistRate) {
  const std::int16_t samples[] = {1, 0, -1, 0};
  EXPECT_THAT(Spectrum(samples), ElementsAre(0, 2, 0, 2));
}

TEST(SpectrumTest, DcAndNyquist) {
  const std::int16_t samples[] = {1, 0, 1, 0};
  EXPECT_THAT(Spectrum(samples), ElementsAre(2, 0, 2, 0));
}

TEST(SpectrumTest, Sine) {
  const std::int16_t samples[] = {0, 1, 0, -1};
  EXPECT_THAT(Spectrum(samples), ElementsAre(0, -2i, 0, 2i));
}

int main(int argc, char** argv) {
  absl::InstallFailureSignalHandler({});
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
