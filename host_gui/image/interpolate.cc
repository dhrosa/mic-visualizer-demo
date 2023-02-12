#include "interpolate.h"

#include <cassert>
#include <deque>

#include "diy/coro/task.h"
#include "qimage_eigen.h"

namespace {

// Linearly interpolate between `a` and `b` according to parameter `t`, which
// must have range [0, 1].
QImage Blend(double t, const QImage& image_a, const QImage& image_b) {
  // We perform blends on 8-bit values using 16-bit fixed-point arithmetic.
  constexpr std::uint16_t max8 = 0xFF;
  const std::uint16_t a_weight = (1.0 - t) * max8;
  const std::uint16_t b_weight = max8 - a_weight;

  QImage image(image_a.size(), image_a.format());
  auto out = EigenView8(image);
  auto a = EigenView8(image_a).template cast<std::uint16_t>() * a_weight;
  auto b = EigenView8(image_b).template cast<std::uint16_t>() * b_weight;
  out = ((a + b) / max8).template cast<std::uint8_t>();
  return image;
}

}  // namespace

AsyncGenerator<QImage> Interpolate(AsyncGenerator<QImage> source,
                                   Rational input_timebase,
                                   Rational output_timebase) {
  if (double(input_timebase) < double(output_timebase)) {
    throw std::logic_error(
        "Input timebase is finer resolution than output timebase.");
  }
  // Input frames N and N+1.
  QImage input_frames[2];
  // Fill initial buffer of input frames.
  for (QImage& initial_frame : input_frames) {
    QImage* frame = co_await source;
    if (frame == nullptr) {
      co_return;
    }
    initial_frame = std::move(*frame);
  }
  std::int64_t output_frame_number = 0;
  for (std::int64_t input_frame_number = 0;; ++input_frame_number) {
    const double input_start_timestamp =
        static_cast<double>(input_timebase * input_frame_number);
    const double input_end_timestamp =
        static_cast<double>(input_timebase * (input_frame_number + 1));
    const double input_duration = input_end_timestamp - input_start_timestamp;
    // Produce as many output frames from the current set of input frames as
    // possible.
    for (;; ++output_frame_number) {
      const double output_start_timestamp =
          static_cast<double>(output_timebase * output_frame_number);
      if (output_start_timestamp > input_end_timestamp) {
        break;
      }
      // Blend parameter with range [0, 1]. Output frames closer in time to the
      // beginning of the input frame will be weighted more towards the first
      // buffered input frame.
      const double t =
          (output_start_timestamp - input_start_timestamp) / input_duration;
      co_yield Blend(t, input_frames[0], input_frames[1]);
    }
    // Read next input frame.
    input_frames[0] = std::move(input_frames[1]);
    QImage* next_input = co_await source;
    if (next_input == nullptr) {
      co_return;
    }
    input_frames[1] = std::move(*next_input);
  }
}
