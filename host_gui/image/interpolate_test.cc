#include "interpolate.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QtGui/QImage>

#include "diy/coro/async_generator.h"

constexpr std::size_t kWidth = 2;
constexpr std::size_t kHeight = 2;

using testing::Optional;

std::optional<std::uint32_t> NextValue(auto& gen) {
  QImage* image = Task(gen).Wait();
  if (image == nullptr) {
    return std::nullopt;
  }
  return image->pixel(0, 0);
}

std::vector<std::uint32_t> FrameValues(AsyncGenerator<QImage> frames) {
  std::vector<std::uint32_t> values;
  while (QImage* image = Task(frames).Wait()) {
    values.push_back(image->pixel(0, 0));
  }
  return values;
}

QImage FilledImage(std::uint32_t value) {
  QImage image(kWidth, kHeight, QImage::Format_ARGB32);
  image.fill(value);
  return image;
}

TEST(InterpolateTest, EmptyInput) {
  auto gen = Interpolate(std::vector<QImage>(), Rational{1, 1}, Rational{1, 1});
  EXPECT_THAT(NextValue(gen), std::nullopt);
}

// At least two frames are needed for interpolation.
TEST(InterpolateTest, SingleFrame) {
  auto gen = Interpolate(std::vector<QImage>({FilledImage(0)}), Rational{1, 1},
                         Rational{1, 1});
  EXPECT_THAT(NextValue(gen), std::nullopt);
}

TEST(InterpolateTest, Blend50) {
  std::vector<QImage> source = {
      FilledImage(100),
      FilledImage(200),
      FilledImage(250),
  };
  auto gen = Interpolate(source, {1, 1}, {1, 2});
  EXPECT_THAT(NextValue(gen), Optional(100));
  EXPECT_THAT(NextValue(gen), Optional(150));
  EXPECT_THAT(NextValue(gen), Optional(200));
  EXPECT_THAT(NextValue(gen), Optional(225));
}
