#include "circular_buffer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::AllOf;
using testing::ElementsAre;
using testing::ElementsAreArray;
using testing::Matcher;
using testing::ResultOf;

// gMock matchers against Eigen array.

auto HasWidth(auto width) {
  return ResultOf(
      "width", [](auto array) { return array.cols(); }, width);
}

auto HasHeight(auto height) {
  return ResultOf(
      "height", [](auto array) { return array.rows(); }, height);
}

auto HasDimensions(auto width, auto height) {
  return AllOf(HasWidth(width), HasHeight(height));
}

auto ArrayElementsAre(std::vector<std::vector<int>> expected) {
  auto as_2d_vector = [](const auto& array) {
    std::vector<std::vector<int>> values;
    for (const auto& row : array.rowwise()) {
      values.emplace_back(row.begin(), row.end());
    }
    return values;
  };
  std::vector<Matcher<std::vector<int>>> row_matchers;
  for (const auto& row : expected) {
    row_matchers.push_back(ElementsAreArray(row));
  }
  return ResultOf("data", as_2d_vector, ElementsAreArray(row_matchers));
}

TEST(CircularBufferTest, Dimensions) {
  CircularBuffer<int> buffer(4, 5);
  EXPECT_EQ(buffer.width(), 4);
  EXPECT_EQ(buffer.columns(), 4);
  EXPECT_EQ(buffer.height(), 5);
  EXPECT_EQ(buffer.rows(), 5);
}

TEST(CircularBufferTest, EmptyBuffer) {
  CircularBuffer<int> buffer(2, 2);
  EXPECT_THAT(buffer.Newer(), ArrayElementsAre({{0, 0}, {0, 0}}));
  EXPECT_THAT(buffer.Older(), ArrayElementsAre({{}, {}}));
}

TEST(CircularBufferTest, Fill) {
  CircularBuffer<int> buffer(2, 2, 86);
  EXPECT_THAT(buffer.Newer(), ArrayElementsAre({{86, 86}, {86, 86}}));
  EXPECT_THAT(buffer.Older(), ArrayElementsAre({{}, {}}));
}

TEST(CircularBufferTest, Append) {
  CircularBuffer<int> buffer(3, 2);

  buffer.AppendColumn(std::span<const int>({1, 2}));
  EXPECT_THAT(buffer.Older(), ArrayElementsAre({{0, 0}, {0, 0}}));
  EXPECT_THAT(buffer.Newer(), ArrayElementsAre({{1}, {2}}));

  buffer.AppendColumn(std::span<const int>({3, 4}));
  EXPECT_THAT(buffer.Older(), ArrayElementsAre({{0}, {0}}));
  EXPECT_THAT(buffer.Newer(), ArrayElementsAre({{1, 3}, {2, 4}}));

  buffer.AppendColumn(std::span<const int>({5, 6}));
  EXPECT_THAT(buffer.Older(), ArrayElementsAre({{}, {}}));
  EXPECT_THAT(buffer.Newer(), ArrayElementsAre({{1, 3, 5}, {2, 4, 6}}));

  // Full loop around; the {1, 2} column should be dropped.
  buffer.AppendColumn(std::span<const int>({7, 8}));
  EXPECT_THAT(buffer.Older(), ArrayElementsAre({{3, 5}, {4, 6}}));
  EXPECT_THAT(buffer.Newer(), ArrayElementsAre({{7}, {8}}));
}
