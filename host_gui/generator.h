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
  using Handle = generator_internal::Handle<T>;

  Generator(Handle handle) : iter_{handle} {}
  ~Generator() { iter_.handle.destroy(); }

  T operator()() { return *++iter_; }

  struct Iterator {
    Handle handle;

    using value_type = T;
    using difference_type = std::ptrdiff_t;

    T& operator*() const { return handle.promise().value; }

    Iterator& operator++() {
      handle.resume();
      if (handle.promise().exception) {
        std::rethrow_exception(handle.promise().exception);
      }
      return *this;
    }

    Iterator& operator++(int) { return this->operator++(); }

    bool operator==(Iterator) const { return handle.done(); }
  };

  Iterator begin() const { return ++iter_; }
  Iterator end() const { return iter_; }

 private:
  mutable Iterator iter_;
};
