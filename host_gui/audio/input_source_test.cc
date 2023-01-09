#include "input_source.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "diy/coro/task.h"

using testing::Gt;
using testing::Pointee;
using testing::SizeIs;

TEST(InputSourceTest, SomeFrames) {
  auto gen = InputSource();

  auto non_empty = Pointee(SizeIs(Gt(0)));

  EXPECT_THAT(Task(gen).Wait(), non_empty);
  EXPECT_THAT(Task(gen).Wait(), non_empty);
  EXPECT_THAT(Task(gen).Wait(), non_empty);
}
