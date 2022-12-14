#include "input_source.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(InputSourceTest, SomeFrames) {
  auto gen = InputSource();

  auto iter = gen.begin().Wait();
  EXPECT_NE(iter, gen.end());
  EXPECT_GT((*iter).size(), 0);
  ++iter;

  EXPECT_NE(iter, gen.end());
  EXPECT_GT((*iter).size(), 0);
  ++iter;

  EXPECT_NE(iter, gen.end());
  EXPECT_GT((*iter).size(), 0);
}
