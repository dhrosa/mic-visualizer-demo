#include "circular_buffer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::ElementsAre;

TEST(CircularBufferTest, Dimensions) {
  CircularBuffer<int> buffer(4, 5);
  EXPECT_EQ(buffer.width(), 4);
  EXPECT_EQ(buffer.columns(), 4);
  EXPECT_EQ(buffer.height(), 5);
  EXPECT_EQ(buffer.rows(), 5);
}

TEST(CircularBufferTest, EmptyBufferOlderBiased) {
  CircularBuffer<int> buffer(2, 2);
  EXPECT_EQ(buffer.Newer().cols(), 0);
  EXPECT_EQ(buffer.Older().cols(), 2);
}

TEST(CircularBufferTest, Fill) {
  CircularBuffer<int> buffer(2, 2, 86);
  EXPECT_THAT(buffer.Older().reshaped(), ElementsAre(86, 86, 86, 86));
}
