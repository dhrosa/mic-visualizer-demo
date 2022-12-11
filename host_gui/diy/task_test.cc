#include "task.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(TaskTest, ReturnValue) {
  bool called = false;
  auto task = [](bool& called) -> Task<int> {
    called = true;
    co_return 4;
  }(called);

  EXPECT_EQ(task.Wait(), 4);
  EXPECT_TRUE(called);
}

TEST(TaskTest, ReturnVoid) {
  bool called = false;
  auto task = [](bool& called) -> Task<> {
    called = true;
    co_return;
  }(called);

  task.Wait();
  EXPECT_TRUE(called);
}
