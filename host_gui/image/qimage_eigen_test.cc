#include "qimage_eigen.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <span>

TEST(QImageEigenTest, Dimensions) {
  QImage image(3, 2, QImage::Format_ARGB32);
  auto view = EigenView(image);
  EXPECT_EQ(view.cols(), 3);
  EXPECT_EQ(view.rows(), 2);
  EXPECT_EQ(6, view.size());

  static_assert(std::same_as<std::uint32_t, decltype(view)::Scalar>);
}

TEST(QImageEigenTest, ScanLinesMatch) {
  QImage image(3, 2, QImage::Format_ARGB32);
  auto view = EigenView(image);

  EXPECT_EQ(reinterpret_cast<std::uint32_t*>(image.scanLine(0)),
            view.row(0).data());

  EXPECT_EQ(reinterpret_cast<std::uint32_t*>(image.scanLine(1)),
            view.row(1).data());
}

TEST(QImageEigenTest, Read) {
  QImage image(3, 2, QImage::Format_ARGB32);
  image.setPixel(0, 0, 1111);
  image.setPixel(2, 1, 2222);

  auto view = EigenView(image);
  EXPECT_EQ(view(0, 0), 1111);
  EXPECT_EQ(view(1, 2), 2222);
}

TEST(QImageEigenTest, Write) {
  QImage image(3, 2, QImage::Format_ARGB32);

  auto view = EigenView(image);
  view(0, 0) = 1111;
  view(1, 2) = 2222;

  EXPECT_EQ(image.pixel(0, 0), 1111);
  EXPECT_EQ(image.pixel(2, 1), 2222);
}
