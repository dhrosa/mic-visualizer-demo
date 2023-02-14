#pragma once

#include <absl/time/time.h>

#include "diy/rational.h"

// Given an expected average output frame rate (`timebase`, measured in frames
// per second), calculates the time that each arriving frame should be rendered
// at to maintain that frame rate.
//
// TODO(dhrosa): This doesn't handle bursty or delayed frame production.
class FrameScheduler {
 public:
  FrameScheduler(Rational timebase) : timebase_(timebase) {}

  absl::Time Schedule(absl::Time arrival_time);

 private:
  const Rational timebase_;
  absl::Time epoch_ = absl::InfinitePast();
  std::int64_t current_frame_number_ = 0;
};

inline absl::Time FrameScheduler::Schedule(absl::Time arrival_time) {
  if (epoch_ == absl::InfinitePast()) {
    epoch_ = arrival_time;
  }
  const absl::Time render_time =
      epoch_ +
      absl::Seconds(static_cast<double>(timebase_ * current_frame_number_));
  ++current_frame_number_;
  return render_time;
}
