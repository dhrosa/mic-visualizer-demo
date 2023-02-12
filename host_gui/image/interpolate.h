#pragma once

#include <absl/time/time.h>

#include <QtGui/QImage>

#include "diy/coro/async_generator.h"
#include "diy/rational.h"

AsyncGenerator<QImage> Interpolate(AsyncGenerator<QImage> source,
                                   Rational input_timebase,
                                   Rational output_timebase);
