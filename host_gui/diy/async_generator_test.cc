#include "async_generator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::ElementsAre;

TEST(AsyncGeneratorTest, Empty) {
  auto gen = []() -> AsyncGenerator<int> { co_return; }();
  auto iter = gen.begin().Wait();
  EXPECT_EQ(iter, gen.end());
}

template <typename T>
Task<std::vector<T>> Materialize(AsyncGenerator<T> gen) {
  std::vector<T> out;
  for (auto iter = co_await gen.begin(); iter != gen.end(); co_await ++iter) {
    out.push_back(*iter);
  }
  co_return out;
}

TEST(AsyncGeneratorTest, Finite) {
  auto gen = []() -> AsyncGenerator<int> {
    co_yield 1;
    co_yield 2;
    co_yield 3;
  };

  EXPECT_THAT(Materialize(gen()).Wait(), ElementsAre(1, 2, 3));
}

TEST(AsyncGeneratorTest, Nested) {
  auto gen_a = []() -> AsyncGenerator<int> {
    co_yield 1;
    co_yield 2;
    co_yield 3;
  };

  auto gen_b = [](AsyncGenerator<int> values) -> AsyncGenerator<int> {
    for (auto iter = co_await values.begin(); iter != values.end();
         co_await ++iter) {
      co_yield *iter * 2;
    }
  };

  EXPECT_THAT(Materialize(gen_b(gen_a())).Wait(), ElementsAre(2, 4, 6));
}

TEST(AsyncGeneratorTest, PipeOperator) {
  auto gen_a = []() -> AsyncGenerator<int> {
    co_yield 1;
    co_yield 2;
    co_yield 3;
  };

  auto gen_b = [](AsyncGenerator<int> values) -> AsyncGenerator<int> {
    for (auto iter = co_await values.begin(); iter != values.end();
         co_await ++iter) {
      co_yield *iter * 2;
    }
  };

  EXPECT_THAT(Materialize(gen_a() | gen_b).Wait(), ElementsAre(2, 4, 6));
}
