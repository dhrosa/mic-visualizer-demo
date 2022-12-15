#include "async_generator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::ElementsAre;

TEST(AsyncGeneratorTest, Empty) {
  auto gen = []() -> AsyncGenerator<int> { co_return; }();
  EXPECT_THAT(gen().Wait(), testing::IsNull());
}

template <typename T>
Task<std::vector<T>> Materialize(AsyncGenerator<T> gen) {
  std::vector<T> out;
  while (T* value = co_await gen) {
    out.push_back(*value);
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
    while (int* value = co_await values) {
      co_yield *value * 2;
    }
  };

  EXPECT_THAT(Materialize(gen_b(gen_a())).Wait(), ElementsAre(2, 4, 6));
}

TEST(AsyncGeneratorTest, ChainAsyncToAsync) {
  auto gen_a = []() -> AsyncGenerator<int> {
    co_yield 1;
    co_yield 2;
    co_yield 3;
  };

  auto gen_b = [](AsyncGenerator<int> values) -> AsyncGenerator<int> {
    while (int* value = co_await values) {
      co_yield *value * 2;
    }
  };

  EXPECT_THAT(Materialize(gen_a() | gen_b).Wait(), ElementsAre(2, 4, 6));
}

TEST(AsyncGeneratorTest, ChainSyncToAsync) {
  auto gen_a = []() -> Generator<int> {
    co_yield 1;
    co_yield 2;
    co_yield 3;
  };

  auto gen_b = [](AsyncGenerator<int> values) -> AsyncGenerator<int> {
    while (int* value = co_await values) {
      co_yield *value * 2;
    }
  };

  EXPECT_THAT(Materialize(gen_a() | gen_b).Wait(), ElementsAre(2, 4, 6));
}
