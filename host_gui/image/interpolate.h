#pragma once

#include <absl/time/time.h>

#include <QtGui/QImage>

#include "diy/coro/async_generator.h"

struct Rational {
  std::int64_t numerator;
  std::int64_t denominator;

  Rational operator*(std::int64_t c) const {
    return {c * numerator, denominator};
  }
  double ToReal() const { return static_cast<double>(numerator) / denominator; }
};

AsyncGenerator<QImage> Interpolate(AsyncGenerator<QImage> source,
                                   Rational input_timebase,
                                   Rational output_timebase);
