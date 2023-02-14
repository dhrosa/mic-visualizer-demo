#include "qimage_aligned.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(QImageAlignedTest, RejectsNonBinaryPowerAlignment) {
  EXPECT_THROW(AlignedQImage(10, 10, 9), std::invalid_argument);
}

TEST(QImageAlignedTest, RejectsAlignmentUnder4) {
  EXPECT_THROW(AlignedQImage(10, 10, 2), std::invalid_argument);
}

std::uintptr_t ScanLineAddress(const QImage& image, int row) {
  return reinterpret_cast<std::uintptr_t>(image.constScanLine(row));
}

TEST(QImageAlignedTest, WidthMultipleOfAlignment) {
  const QImage image = AlignedQImage(512, 3, 64);
  EXPECT_EQ(image.width(), 512);
  EXPECT_EQ(image.height(), 3);
  EXPECT_EQ(image.bytesPerLine(), 2048);
  EXPECT_EQ(image.sizeInBytes(), 6144);

  EXPECT_EQ(ScanLineAddress(image, 0) % 64, 0);
  EXPECT_EQ(ScanLineAddress(image, 1), ScanLineAddress(image, 0) + 2048);
  EXPECT_EQ(ScanLineAddress(image, 2), ScanLineAddress(image, 1) + 2048);
}

// A 500-pixel wide image should be over-allocated to 512-pixel scan lines.
TEST(QImageAlignedTest, WidthNotMultipleOfAlignment) {
  const QImage image = AlignedQImage(500, 3, 64);
  EXPECT_EQ(image.width(), 500);
  EXPECT_EQ(image.height(), 3);
  EXPECT_EQ(image.bytesPerLine(), 2048);
  EXPECT_EQ(image.sizeInBytes(), 6144);

  EXPECT_EQ(ScanLineAddress(image, 0) % 64, 0);
  EXPECT_EQ(ScanLineAddress(image, 1), ScanLineAddress(image, 0) + 2048);
  EXPECT_EQ(ScanLineAddress(image, 2), ScanLineAddress(image, 1) + 2048);
}
