#include "input_source.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(InputSourceTest, SomeFrames) {
  auto gen = InputSource();

  EXPECT_TRUE(gen.Advance().Wait());
  EXPECT_GT(gen.Value().size(), 0);

  EXPECT_TRUE(gen.Advance().Wait());
  EXPECT_GT(gen.Value().size(), 0);

  EXPECT_TRUE(gen.Advance().Wait());
  EXPECT_GT(gen.Value().size(), 0);
}
