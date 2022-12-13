#include "qaudio_source_coro.h"

#include <QDebug>
#include <coroutine>
#include <utility>

namespace {
QAudioFormat AudioFormat() {
  QAudioFormat format;
  format.setChannelCount(1);
  format.setSampleFormat(QAudioFormat::Int16);
  format.setSampleRate(24'000);
  return format;
}
}  // namespace

QAudioSourceCoro::QAudioSourceCoro(QAudioDevice audio_device)
    : source_(audio_device, AudioFormat()) {
  QObject::connect(&source_, &QAudioSource::stateChanged,
                   [this](QAudio::State state) { OnStateChange(state); });
  device_ = source_.start();
}

struct QAudioSourceCoro::StateChangeAwaiter {
  QAudioSourceCoro* coro;
  QAudio::State state;
  std::coroutine_handle<> waiting;

  bool await_ready() { return coro->source_.state() != QAudio::IdleState; }

  void await_suspend(std::coroutine_handle<> waiter) {
    waiting = waiter;
    absl::MutexLock lock(&coro->mutex_);
    coro->waiter_ = this;
  }

  QAudio::State await_resume() { return state; }
};

void QAudioSourceCoro::OnStateChange(QAudio::State state) {
  if (state == QAudio::IdleState) {
    return;
  }
  StateChangeAwaiter* waiter;
  {
    absl::MutexLock lock(&mutex_);
    waiter = std::exchange(waiter_, nullptr);
  }
  if (!waiter) {
    return;
  }
  waiter->state = state;
  waiter->waiting.resume();
}

AsyncGenerator<Buffer<std::int16_t>> QAudioSourceCoro::Frames() {
  while (true) {
    QAudio::State state = co_await StateChangeAwaiter{this};
    if (state == QAudio::StoppedState) {
      if (source_.error() == QAudio::NoError) {
        co_return;
      }
      throw std::runtime_error("Error while reading frame: " +
                               QDebug::toString(source_.error()).toStdString());
    }
    if (state != QAudio::ActiveState) {
      throw std::runtime_error("Unexpected state while reading frame: " +
                               QDebug::toString(state).toStdString());
    }
    // Data is available to read.
    QByteArray raw_data = device_->readAll();
    if (raw_data.size() % 2 != 0) {
      throw std::runtime_error("Non-even number of bytes in frame: " +
                               std::to_string(raw_data.size()));
    }
    auto* ptr = reinterpret_cast<std::int16_t*>(raw_data.data());
    const std::size_t count = raw_data.size() / 2;
    co_yield Buffer<std::int16_t>(std::span<std::int16_t>(ptr, count),
                                  [raw_data = std::move(raw_data)]() {});
  }
}
