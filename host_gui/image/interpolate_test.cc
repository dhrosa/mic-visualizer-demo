#include "interpolate.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QtGui/QImage>

#include "diy/coro/async_generator.h"

constexpr std::size_t kWidth = 2;
constexpr std::size_t kHeight = 2;

using testing::ElementsAre;
using testing::IsEmpty;
using testing::IsNull;
using testing::Pointee;

QImage FilledImage(std::uint32_t value) {
  QImage image(kWidth, kHeight, QImage::Format_ARGB32);
  image.fill(value);
  return image;
}

std::uint32_t Value(const QImage& image) { return image.pixel(0, 0); }

TEST(InterpolateTest, EmptyInput) {
  auto gen = Interpolate(std::vector<QImage>(), Rational{1, 1}, Rational{1, 1});
  EXPECT_THAT(gen.ToVector(), IsEmpty());
}

// At least two frames are needed for interpolation.
TEST(InterpolateTest, SingleFrame) {
  std::vector<QImage> source = {
      FilledImage(0),
  };
  auto gen = Interpolate(source, Rational{1, 1}, Rational{1, 1});
  EXPECT_THAT(gen.ToVector(), IsEmpty());
}

TEST(InterpolateTest, Blend50) {
  std::vector<QImage> source = {
      FilledImage(100),
      FilledImage(200),
      FilledImage(250),
  };
  EXPECT_THAT(Interpolate(source, {1, 1}, {1, 2}).Map(Value).ToVector(),
              ElementsAre(100, 150, 200, 225, 250));
}
