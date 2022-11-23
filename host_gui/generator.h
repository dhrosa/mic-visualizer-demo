// Largely follows the Generator example from
// https://en.cppreference.com/w/cpp/language/coroutines

#pragma once

#include <absl/cleanup/cleanup.h>

#include <coroutine>
#include <exception>
#include <iterator>
#include <memory>

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

template <typename T>
struct HandleCleanup {
  Handle<T> handle;

  HandleCleanup(Handle<T> handle) : handle(handle) {}
  ~HandleCleanup() { handle.destroy(); }
};
}  // namespace generator_internal

template <typename T>
class Generator {
 public:
  using promise_type = generator_internal::Promise<T>;
  using value_type = T;
  using Handle = generator_internal::Handle<T>;

  Generator() = default;

  Generator(Handle handle)
      : shared_handle_(
            std::make_shared<generator_internal::HandleCleanup<T>>(handle)),
        iter_{handle} {}

  T operator()() { return *++iter_; }

  struct Iterator {
    Handle handle;

    using value_type = T;
    // Needed for std::weakly_incrementable.
    using difference_type = std::ptrdiff_t;
    // Without this std::iterator_traits assumes the category is
    // std::forward_iterator_tag, which supports multi-pass. This
    // iterator can only be passed over once.
    using iterator_category = std::input_iterator_tag;

    T& operator*() const { return handle.promise().value; }

    T* operator->() const { return &handle.promise().value(); }

    Iterator& operator++() {
      handle.resume();
      if (handle.promise().exception) {
        std::rethrow_exception(handle.promise().exception);
      }
      return *this;
    }

    // Needed for std::weakly_incrementable.
    Iterator& operator++(int) { return this->operator++(); }

    bool operator==(Iterator) const { return handle.done(); }
  };

  constexpr Iterator begin() const { return ++iter_; }
  constexpr Iterator end() const { return iter_; }

 private:
  std::shared_ptr<generator_internal::HandleCleanup<T>> shared_handle_;
  mutable Iterator iter_;
};

template <typename T>
inline constexpr bool std::ranges::enable_view<Generator<T>> = true;

// template <typename T>
// inline constexpr bool std::ranges::enable_borrowed_range<Generator<T>> =
// true;
