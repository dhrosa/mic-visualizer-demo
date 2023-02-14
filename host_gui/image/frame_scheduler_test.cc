#include "frame_scheduler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ranges>
#include <span>

using testing::ElementsAre;

std::vector<absl::Time> ScheduleFrames(
    FrameScheduler& scheduler, int count,
    absl::Time first_arrival_time = absl::UnixEpoch()) {
  std::vector<absl::Time> times(count, first_arrival_time);
  for (absl::Time& time : times) {
    time = scheduler.Schedule(time);
  }
  return times;
}

TEST(FrameSchedulerTest, FramesEvenlySpaced) {
  FrameScheduler scheduler({1, 4});
  EXPECT_THAT(ScheduleFrames(scheduler, 5),
              ElementsAre(absl::FromUnixMillis(0), absl::FromUnixMillis(250),
                          absl::FromUnixMillis(500), absl::FromUnixMillis(750),
                          absl::FromUnixMillis(1000)));
}

TEST(FrameSchedulerTest, FramesOffsetByFirstFrame) {
  FrameScheduler scheduler({1, 4});
  EXPECT_THAT(ScheduleFrames(scheduler, 5, absl::FromUnixMillis(3)),
              ElementsAre(absl::FromUnixMillis(3), absl::FromUnixMillis(253),
                          absl::FromUnixMillis(503), absl::FromUnixMillis(753),
                          absl::FromUnixMillis(1003)));
}
