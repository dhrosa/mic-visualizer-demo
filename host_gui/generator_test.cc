#include "generator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iterator>
#include <ranges>

TEST(GeneratorTest, Iota) {
  auto iota = []() -> Generator<int> {
    for (int i = 0; true; ++i) {
      co_yield i;
    }
  };

  auto gen = iota();

  EXPECT_EQ(gen(), 0);
  EXPECT_EQ(gen(), 1);
  EXPECT_EQ(gen(), 2);
}

TEST(GeneratorTest, ExceptionProagated) {
  auto f = []() -> Generator<int> {
    co_yield 0;
    co_yield 1;
    throw std::runtime_error("fake exception");
  };

  auto gen = f();
  EXPECT_EQ(gen(), 0);
  EXPECT_EQ(gen(), 1);
  EXPECT_THROW(gen(), std::runtime_error);
}

TEST(IteratorTest, Finite) {
  auto gen = []() -> Generator<int> { co_yield 0; }();

  // EXPECT_THAT(std::vector(gen.begin(), gen.end()), testing::ElementsAre(0));
  auto iter = gen.begin();
  EXPECT_FALSE(iter == gen.end());
  EXPECT_EQ(0, *iter);
  ++iter;
  EXPECT_TRUE(iter == gen.end());
}
