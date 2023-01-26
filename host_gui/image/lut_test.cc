#include "lut.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ranges>

using testing::ElementsAre;

// Matcher for 2D Eigen array values.
auto ArrayElementsAre(std::vector<std::vector<int>> expected) {
  using testing::ElementsAreArray;
  using testing::Matcher;
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

TEST(ToIndexedTest, MapsValues) {
  // Values below 1.0 should be mapped to 0. Values above 2.0 should map to 255.
  // Values in-between should be mapped proportionally.
  const double min = 1.0;
  const double max = 2.0;
  const std::vector<double> values = {0.75, 1.0, 1.25, 1.5, 2.0, 2.5};
  std::vector<std::uint8_t> indexed(values.size());

  ToIndexed(values, indexed, min, max);
  EXPECT_THAT(indexed, ElementsAre(0, 0, 64, 128, 255, 255));
}

TEST(LutMapTest, LinearMapping) {
  using namespace Eigen;

  // Lut contains entries that will look like: {0, 1001, 2002, ..., 154154,
  // 155155, ..., 255255}. This pattern is to ensure that the implementation
  // isn't just accidentally copying the source over to the destination, or
  // misaligning bits of the output.
  std::array<std::uint32_t, 256> lut;
  for (int i = 0; i < lut.size(); ++i) {
    lut[i] = 1001 * i;
  }

  const Array<std::uint8_t, Dynamic, Dynamic> source(
      {{0, 1, 2}, {253, 254, 255}});
  Array<std::uint32_t, Dynamic, Dynamic> dest(2, 3);
  LutMap(source, dest, lut);

  EXPECT_THAT(dest,
              ArrayElementsAre({{0, 1001, 2002}, {253253, 254254, 255255}}));
}
