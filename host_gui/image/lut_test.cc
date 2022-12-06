#include "lut.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ranges>

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

auto ConstantLut(std::uint32_t val) {
  std::array<std::uint32_t, 256> lut;
  std::ranges::fill(lut, val);
  return lut;
}

constexpr auto LinearLut(std::uint32_t start) {
  std::array<std::uint32_t, 256> lut;
  std::ranges::copy_n(std::ranges::iota_view<std::uint32_t>(start).begin(), 256,
                      lut.begin());
  return lut;
}

TEST(LutTest, ConstantLut) {
  using namespace Eigen;
  const Array<double, Dynamic, Dynamic> source({{1, 2, 3}, {4, 5, 6}});
  Array<std::uint32_t, Dynamic, Dynamic> dest(2, 3);
  LutMap(source, dest, ConstantLut(23), 0.0, 1.0);

  EXPECT_THAT(dest, ArrayElementsAre({{23, 23, 23}, {23, 23, 23}}));
}

TEST(LutTest, LinearMapping) {
  using namespace Eigen;
  const Array<double, Dynamic, Dynamic> source(
      {{0.75, 1.0, 1.25}, {1.5, 2.0, 2.5}});
  Array<std::uint32_t, Dynamic, Dynamic> dest(2, 3);
  LutMap(source, dest, LinearLut(1000), 1.0, 2.0);

  EXPECT_THAT(dest, ArrayElementsAre({{1000, 1000, 1064}, {1128, 1255, 1255}}));
}
