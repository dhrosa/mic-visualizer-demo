#pragma once

#include <stdexcept>

#include "handle.h"
#include "task.h"

template <typename T>
class AsyncGenerator {
  struct Promise;
  struct Iterator;
  struct Sentinel {};

 public:
  using promise_type = Promise;
  using value_type = T;

  explicit AsyncGenerator(Handle handle) : handle_(std::move(handle)) {}

  Task<Iterator> begin();
  auto end() { return Sentinel{}; }

 private:
  Handle handle_;
};

template <typename T>
auto AsyncGenerator<T>::begin() -> Task<Iterator> {
  handle_->resume();
  co_return Iterator{this};
}

template <typename T>
struct AsyncGenerator<T>::Promise {
  T value;
  std::exception_ptr exception;

  AsyncGenerator<T> get_return_object() {
    return AsyncGenerator<T>(
        Handle(std::coroutine_handle<Promise>::from_promise(*this)));
  }

  auto initial_suspend() { return std::suspend_always{}; }
  auto final_suspend() noexcept { return std::suspend_always{}; }
  void unhandled_exception() { exception = std::current_exception(); }
  void return_void() {}

  template <std::convertible_to<T> U>
  auto yield_value(U&& new_value) {
    value = std::forward<U>(new_value);
    return std::suspend_always{};
  }

  T& YieldOrThrow() {
    if (exception) {
      std::rethrow_exception(exception);
    }
    return value;
  }
};

template <typename T>
struct AsyncGenerator<T>::Iterator {
  AsyncGenerator<T>* generator;

  bool operator==([[maybe_unused]] Sentinel s) const {
    return generator->handle_->done();
  }

  T& operator*() {
    return generator->handle_.template promise<Promise>().YieldOrThrow();
  }

  Task<Iterator> operator++() {
    if (generator->handle_->done()) {
      throw std::out_of_range("AsyncGenerator exhausted.");
    }
    generator->handle_->resume();
    co_return *this;
  }
};

template <typename T>
constexpr bool kIsAsyncGenerator = false;

template <typename T>
constexpr bool kIsAsyncGenerator<AsyncGenerator<T>> = true;

template <typename T>
concept IsAsyncGenerator = kIsAsyncGenerator<T>;

template <typename Producer, typename Consumer>
concept Chainable = kIsAsyncGenerator<Producer> &&
                    requires(Producer producer, Consumer consumer) {
                      { consumer(std::move(producer)) } -> IsAsyncGenerator;
                    };

template <typename P, typename C>
auto operator|(P&& p, C&& c)
  requires Chainable<P, C>
{
  return c(std::move(p));
}
