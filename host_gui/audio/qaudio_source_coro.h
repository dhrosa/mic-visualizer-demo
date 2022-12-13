#pragma once

#include <absl/synchronization/mutex.h>

#include <QAudioSource>
#include <QMediaDevices>
#include <coroutine>
#include <cstdint>

#include "diy/async_generator.h"
#include "diy/buffer.h"

class QAudioSourceCoro {
 public:
  QAudioSourceCoro(
      QAudioDevice audio_device = QMediaDevices::defaultAudioInput());

  AsyncGenerator<Buffer<std::int16_t>> Frames();

 private:
  struct StateChangeAwaiter;

  void OnStateChange(QAudio::State state);

  QAudioSource source_;
  QIODevice* device_;

  absl::Mutex mutex_;
  StateChangeAwaiter* waiter_ ABSL_GUARDED_BY(mutex_);
};
