#include "model.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(ModelTest, FrequencyBins) {
  Model model({.sample_rate = 10.0, .fft_window_size = 16});
  EXPECT_EQ(model.FrequencyBin(0), 0.0);
  EXPECT_EQ(model.FrequencyBin(4), 2.5);
  EXPECT_EQ(model.FrequencyBin(8), 5.0);
}

TEST(ModelTest, TimeDelta) {
  Model model({.sample_rate = 10.0, .fft_window_size = 16});
  EXPECT_EQ(model.TimeDelta(0), absl::ZeroDuration());
  // 16 samples per window @ 10Hz
  EXPECT_EQ(model.TimeDelta(1), absl::Milliseconds(1600));
  EXPECT_EQ(model.TimeDelta(10), absl::Seconds(16));
}
