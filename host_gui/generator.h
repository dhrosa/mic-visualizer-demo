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
  using value_type = T;

  Generator(generator_internal::Handle<T> handle) : handle_(handle) {}
  ~Generator() { handle_.destroy(); }

  bool Done() const {
    FillIfNeeded();
    return !full_ && handle_.done();
  }

  explicit operator bool() const { return !Done(); }

  T operator()() const {
    FillIfNeeded();
    full_ = false;
    return std::move(handle_.promise().value);
  }

  struct Iterator {
    const Generator<T>* generator;

    using value_type = T;
    using difference_type = std::ptrdiff_t;

    T& operator*() const { return generator->handle_.promise().value; }

    Iterator& operator++() {
      generator->handle_.resume();
      return *this;
    }

    Iterator& operator++(int) { return this->operator++(); }

    bool operator==(Iterator) const { return generator->handle_.done(); }
  };

  Iterator begin() const { return {this}; }
  Iterator end() const { return begin(); }

 private:
  void FillIfNeeded() const {
    if (full_ || handle_.done()) {
      return;
    }
    handle_.resume();
    if (handle_.promise().exception) {
      std::rethrow_exception(handle_.promise().exception);
    }
    if (handle_.done()) {
      return;
    }
    full_ = true;
  }

  generator_internal::Handle<T> handle_;
  mutable bool full_ = false;
};
