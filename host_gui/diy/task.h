#pragma once

#include <coroutine>
#include <exception>
#include <type_traits>
#include <utility>

class Handle {
 public:
  Handle() = default;
  explicit Handle(std::coroutine_handle<> handle) : handle_(handle) {}
  Handle(Handle&& other) noexcept { *this = std::move(other); }

  Handle& operator=(Handle&& other) noexcept {
    handle_ = std::exchange(other.handle_, nullptr);
    return *this;
  }

  ~Handle() {
    if (handle_) {
      handle_.destroy();
    }
  }

  template <typename P>
  decltype(auto) promise() const noexcept {
    return std::coroutine_handle<P>::from_address(handle_.address()).promise();
  }

  std::coroutine_handle<> get() const noexcept { return handle_; }

  std::coroutine_handle<>* operator->() noexcept { return &handle_; }

 private:
  std::coroutine_handle<> handle_;
};

template <typename T = void>
class Task {
  struct Promise;
  struct Awaiter;

 public:
  using promise_type = Promise;

  Task(Handle handle) noexcept : handle_(std::move(handle)) {}
  Task(Task&& other) = default;
  Task& operator=(Task&& other) = default;
  ~Task() = default;

  T Wait() {
    handle_->resume();
    Promise& promise = handle_.template promise<Promise>();
    if (promise.exception) {
      std::rethrow_exception(promise.exception);
    }
    return std::move(promise.final_value);
  }

  auto operator co_await() { return Awaiter{this}; }

 private:
  Handle handle_;
};

template <typename T>
struct Task<T>::Promise {
  T final_value;
  std::exception_ptr exception;
  std::coroutine_handle<> next;

  Task<T> get_return_object() {
    auto handle = std::coroutine_handle<Promise>::from_promise(*this);
    return Task<T>(Handle(handle));
  }

  auto initial_suspend() noexcept { return std::suspend_always{}; }
  auto final_suspend() noexcept { return std::suspend_always{}; }
  void unhandled_exception() { exception = std::current_exception(); }
  void return_value(T value) {
    final_value = value;
    if (next) {
      next.resume();
    }
  }
};

template <>
class Task<void> {
  struct Promise;

 public:
  using promise_type = Promise;
  Task(Handle handle) noexcept : handle_(std::move(handle)) {}
  Task(Task&& other) = default;
  Task& operator=(Task&& other) = default;
  ~Task() = default;

  void Wait();

 private:
  Handle handle_;
};

struct Task<void>::Promise {
  std::exception_ptr exception;

  Task<void> get_return_object() {
    auto handle = std::coroutine_handle<Promise>::from_promise(*this);
    return Task<void>(Handle(handle));
  }

  auto initial_suspend() noexcept { return std::suspend_always{}; }
  auto final_suspend() noexcept { return std::suspend_always{}; }
  void unhandled_exception() { exception = std::current_exception(); }
  void return_void() {}
};

void Task<void>::Wait() {
  handle_->resume();
  Promise& promise = handle_.template promise<Promise>();
  if (promise.exception) {
    std::rethrow_exception(promise.exception);
  }
}

template <typename T>
struct Task<T>::Awaiter {
  Task<T>* task;

  bool await_ready() { return false; }

  std::coroutine_handle<> await_suspend(std::coroutine_handle<> current) {
    task->handle_.template promise<Promise>().next = current;
    return task->handle_.get();
  }

  T await_resume() {
    Promise& promise = task->handle_.template promise<Promise>();
    return std::move(promise.final_value);
  }
};
