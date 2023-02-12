#include "rational.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(RationalTest, Print) {
  EXPECT_EQ(testing::PrintToString(Rational{3, 2}), "3/2");
}

TEST(RationalTest, Multiplication) {
  // I don't know why by clang stalls indefinitely by just inlining these
  // variables.
  const auto a = Rational{2, 3} * 5;
  const auto b = Rational{10, 3};
  EXPECT_EQ(a, b);
}

TEST(RationalTest, Double) {
  EXPECT_EQ(static_cast<double>(Rational{1, 2}), 0.5);
}
