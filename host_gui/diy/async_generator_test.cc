#include "async_generator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(AsyncGeneratorTest, Empty) {
  auto gen = []() -> AsyncGenerator<int> { co_return; }();

  auto iter = gen.begin().Wait();
  EXPECT_EQ(iter, gen.end());
}
