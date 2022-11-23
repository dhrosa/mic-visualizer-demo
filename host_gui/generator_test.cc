#include "generator.h"

#include <absl/debugging/failure_signal_handler.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iterator>
#include <ranges>

using testing::ElementsAre;

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

auto ToVector(auto&& gen) { return std::vector(gen.begin(), gen.end()); }

TEST(IteratorTest, Finite) {
  auto gen = []() -> Generator<int> { co_yield 0; }();

  EXPECT_THAT(ToVector(gen), ElementsAre(0));
}

TEST(IteratorTest, Iota) {
  auto gen = []() -> Generator<int> {
    for (int i = 0; true; ++i) {
      co_yield i;
    }
  }();

  auto r = std::move(gen) | std::views::take(1);
}

int main(int argc, char** argv) {
  absl::InstallFailureSignalHandler({});
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
