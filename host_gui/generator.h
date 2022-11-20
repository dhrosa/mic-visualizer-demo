// Largely follows the Generator example from
// https://en.cppreference.com/w/cpp/language/coroutines

#pragma once

#include <coroutine>
#include <exception>

template <typename T>
class Generator;

namespace generator_internal {
template <typename T>
class Promise;
template <typename T>
using Handle = std::coroutine_handle<Promise<T>>;

template <typename T>
struct Promise {
  T value;
  std::exception_ptr exception;

  Generator<T> get_return_object() {
    return Generator<T>(Handle<T>::from_promise(*this));
  }

  std::suspend_always initial_suspend() { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }
  void unhandled_exception() { exception = std::current_exception(); }

  template <std::convertible_to<T> U>
  std::suspend_always yield_value(U&& new_value) {
    value = std::forward<U>(new_value);
    return {};
  }

  void return_void() {}
};
}  // namespace generator_internal

template <typename T>
class Generator {
 public:
  using promise_type = generator_internal::Promise<T>;

  Generator(generator_internal::Handle<T> handle) : handle_(handle) {}
  ~Generator() { handle_.destroy(); }

  explicit operator bool() {
    FillIfNeeded();
    return !handle_.done();
  }

  T operator()() {
    FillIfNeeded();
    full_ = false;
    return std::move(handle_.promise().value);
  }

 private:
  void FillIfNeeded() {
    if (full_) {
      return;
    }
    handle_.resume();
    if (handle_.promise().exception) {
      std::rethrow_exception(handle_.promise().exception);
    }
    full_ = true;
  }

  generator_internal::Handle<T> handle_;
  bool full_ = false;
};
