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

TEST(TaskTest, Chain) {
  auto task_a = []() -> Task<int> { co_return 1; };
  auto task_b = [](Task<int> a) -> Task<int> {
    int val_a = co_await a;
    co_return val_a + 2;
  };

  EXPECT_EQ(task_b(task_a()).Wait(), 3);
}
