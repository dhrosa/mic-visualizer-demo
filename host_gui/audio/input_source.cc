#include "input_source.h"

#define MINIAUDIO_IMPLEMENTATION
#include <absl/cleanup/cleanup.h>
#include <miniaudio.h>

#include <iostream>
#include <ranges>

namespace {
struct Awaiter {
  std::coroutine_handle<> waiting;
  std::span<const std::int16_t> input;

  bool await_ready() { return false; }

  void await_suspend(std::coroutine_handle<> waiting) {
    this->waiting = waiting;
  };

  Buffer<std::int16_t> await_resume() {
    waiting = {};
    auto buffer = Buffer<std::int16_t>::Uninitialized(input.size());
    std::ranges::copy(input, buffer.begin());
    return buffer;
  }
};
}  // namespace

AsyncGenerator<Buffer<std::int16_t>> InputSource() {
  Awaiter awaiter;

  ma_device_config config = ma_device_config_init(ma_device_type_capture);
  config.capture.format = ma_format_s16;
  config.capture.channels = 1;
  config.sampleRate = 24'000;
  config.noFixedSizedCallback = true;
  config.pUserData = &awaiter;
  config.dataCallback = +[](ma_device* device, [[maybe_unused]] void* output,
                            const void* input, ma_uint32 size) {
    auto& awaiter = *static_cast<Awaiter*>(device->pUserData);
    awaiter.input = std::span(static_cast<const std::int16_t*>(input), size);
    if (awaiter.waiting) {
      awaiter.waiting.resume();
    }
  };

  ma_device device;

  if (const ma_result init_result = ma_device_init(nullptr, &config, &device);
      init_result != MA_SUCCESS) {
    throw std::runtime_error("Failed to initialize audio input.");
  }
  auto uninit = absl::MakeCleanup([&] { ma_device_uninit(&device); });
  if (const ma_result start_result = ma_device_start(&device);
      start_result != MA_SUCCESS) {
    throw std::runtime_error("Failed to start audio input capture.");
  }

  while (true) {
    co_yield (co_await awaiter);
  }
  co_return;
}
