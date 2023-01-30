#include "qimage_eigen.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::ElementsAre;

TEST(EigenViewTest, Dimensions) {
  QImage image(3, 2, QImage::Format_ARGB32);
  auto view = EigenView(image);
  EXPECT_EQ(view.cols(), 3);
  EXPECT_EQ(view.rows(), 2);
  EXPECT_EQ(6, view.size());

  static_assert(std::same_as<std::uint32_t, decltype(view)::Scalar>);
}

TEST(EigenViewTest, ScanLinesMatch) {
  QImage image(3, 2, QImage::Format_ARGB32);
  auto view = EigenView(image);

  EXPECT_EQ(reinterpret_cast<std::uint32_t*>(image.scanLine(0)),
            view.row(0).data());

  EXPECT_EQ(reinterpret_cast<std::uint32_t*>(image.scanLine(1)),
            view.row(1).data());
}

TEST(EigenViewTest, Read) {
  QImage image(3, 2, QImage::Format_ARGB32);
  image.setPixel(0, 0, 1111);
  image.setPixel(2, 1, 2222);

  auto view = EigenView(image);
  EXPECT_EQ(view(0, 0), 1111);
  EXPECT_EQ(view(1, 2), 2222);
}

TEST(EigenViewTest, Read8) {
  QImage image(2, 2, QImage::Format_ARGB32);
  image.setPixel(0, 0, 0x03'02'01'00);
  image.setPixel(1, 0, 0x07'06'05'04);
  image.setPixel(0, 1, 0x0B'0A'09'08);
  image.setPixel(1, 1, 0x0F'0E'0D'0C);

  auto view = EigenView8(image);
  EXPECT_THAT(view.row(0), ElementsAre(0, 1, 2, 3, 4, 5, 6, 7));
  EXPECT_THAT(view.row(1), ElementsAre(8, 9, 10, 11, 12, 13, 14, 15));
}

TEST(EigenViewTest, Write8) {
  QImage image(2, 2, QImage::Format_ARGB32);

  auto view = EigenView8(image);
  view.row(0) << 0, 1, 2, 3, 4, 5, 6, 7;
  view.row(1) << 8, 9, 10, 11, 12, 13, 14, 15;

  EXPECT_EQ(image.pixel(0, 0), 0x03'02'01'00);
  EXPECT_EQ(image.pixel(1, 0), 0x07'06'05'04);
  EXPECT_EQ(image.pixel(0, 1), 0x0B'0A'09'08);
  EXPECT_EQ(image.pixel(1, 1), 0x0F'0E'0D'0C);
}

TEST(EigenViewTest, ReadConst) {
  QImage image(3, 2, QImage::Format_ARGB32);
  image.setPixel(0, 0, 1111);
  image.setPixel(2, 1, 2222);

  QImage image_copy = image;

  auto view = EigenView(std::as_const(image));
  EXPECT_EQ(view(0, 0), 1111);
  EXPECT_EQ(view(1, 2), 2222);

  // Creating a const view should not have triggered a copy-on-write detach.
  EXPECT_EQ(image.constBits(), image_copy.constBits());
}

TEST(EigenViewTest, Write) {
  QImage image(3, 2, QImage::Format_ARGB32);

  auto view = EigenView(image);
  view(0, 0) = 1111;
  view(1, 2) = 2222;

  EXPECT_EQ(image.pixel(0, 0), 1111);
  EXPECT_EQ(image.pixel(2, 1), 2222);
}
