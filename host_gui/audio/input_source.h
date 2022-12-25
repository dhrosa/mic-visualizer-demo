#pragma once

#include "diy/buffer.h"
#include "diy/coro/async_generator.h"

AsyncGenerator<Buffer<std::int16_t>> InputSource();
