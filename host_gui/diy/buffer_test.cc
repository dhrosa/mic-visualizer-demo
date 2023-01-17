#include "buffer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <concepts>
#include <ranges>
#include <type_traits>

using testing::ElementsAre;

static_assert(std::is_nothrow_default_constructible_v<Buffer<int>>);
static_assert(std::is_nothrow_default_constructible_v<Buffer<const int>>);

static_assert(std::is_nothrow_move_constructible_v<Buffer<int>>);
static_assert(std::is_nothrow_move_constructible_v<Buffer<const int>>);

static_assert(!std::copy_constructible<Buffer<int>>);
static_assert(!std::copy_constructible<Buffer<const int>>);

static_assert(std::ranges::range<Buffer<int>>);
static_assert(std::ranges::range<Buffer<const int>>);

static_assert(std::ranges::contiguous_range<Buffer<int>>);
static_assert(std::ranges::contiguous_range<Buffer<const int>>);

static_assert(std::ranges::output_range<Buffer<int>, int>);
static_assert(!std::ranges::output_range<Buffer<const int>, const int>);

static_assert(std::ranges::sized_range<Buffer<int>>);
static_assert(std::ranges::sized_range<Buffer<const int>>);

static_assert(std::ranges::viewable_range<Buffer<int>>);
static_assert(std::ranges::viewable_range<Buffer<const int>>);

TEST(BufferTest, Empty) { EXPECT_TRUE(Buffer<int>().empty()); }

TEST(BufferTest, NoOpCleanup) {
  const int values[] = {1, 2, 3};
  EXPECT_THAT(Buffer<const int>(values, [] {}), ElementsAre(1, 2, 3));
}

TEST(BufferTest, CleanupCalledOnce) {
  bool cleanup_called = false;

  const int values[] = {1, 2, 3};
  {
    auto buffer_a = Buffer<const int>(values, [&] { cleanup_called = true; });
    EXPECT_FALSE(cleanup_called);
    EXPECT_THAT(buffer_a, ElementsAre(1, 2, 3));

    auto buffer_b = std::move(buffer_a);
    EXPECT_FALSE(cleanup_called);
    EXPECT_THAT(buffer_b, ElementsAre(1, 2, 3));
  }
  EXPECT_TRUE(cleanup_called);
}

TEST(BufferTest, Mutation) {
  int values[] = {1, 2, 3};
  auto buffer = Buffer<int>(values, [] {});
  buffer[0] = 4;
  buffer[1] = 5;
  buffer[2] = 6;

  EXPECT_THAT(buffer, ElementsAre(4, 5, 6));
  EXPECT_THAT(values, ElementsAre(4, 5, 6));
}

TEST(BufferTest, Adopt) {
  auto buffer = AdoptAsBuffer(std::vector<int>({1, 2, 3}));
  EXPECT_THAT(buffer, ElementsAre(1, 2, 3));
}
