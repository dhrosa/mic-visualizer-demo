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

class TaskBase {
 public:
  TaskBase() = default;
  TaskBase(Handle handle) noexcept : handle_(std::move(handle)) {}
  TaskBase(TaskBase&& other) noexcept = default;
  TaskBase& operator=(TaskBase&& other) noexcept = default;
  ~TaskBase() = default;

 protected:
  struct PromiseBase;
  template <typename P>
  struct Awaiter;

  template <typename P>
  decltype(auto) Wait() {
    handle_->resume();
    auto& promise = handle_.template promise<P>();
    return promise.ReturnOrThrow();
  }

  Handle handle_;
};

struct TaskBase::PromiseBase {
  std::exception_ptr exception;
  std::coroutine_handle<> next;

  auto initial_suspend() noexcept { return std::suspend_always{}; }
  auto final_suspend() noexcept { return std::suspend_always{}; }
  void unhandled_exception() { exception = std::current_exception(); }

  void MaybeRethrow() {
    if (exception) {
      std::rethrow_exception(exception);
    }
  }
};

template <typename P>
struct TaskBase::Awaiter {
  TaskBase* task;

  bool await_ready() const noexcept { return false; }

  std::coroutine_handle<> await_suspend(std::coroutine_handle<> current) {
    task->handle_.template promise<P>().next = current;
    return task->handle_.get();
  }

  auto await_resume() {
    auto& promise = task->handle_.template promise<P>();
    return promise.ReturnOrThrow();
  }
};

template <typename T = void>
class Task : public TaskBase {
  struct Promise;

 public:
  using promise_type = Promise;

  using TaskBase::TaskBase;
  using TaskBase::operator=;

  T Wait() { return TaskBase::template Wait<Promise>(); }

  auto operator co_await() { return Awaiter<Promise>{this}; }
};

template <typename T>
struct Task<T>::Promise : public TaskBase::PromiseBase {
  T final_value;

  Task<T> get_return_object() {
    auto handle = std::coroutine_handle<Promise>::from_promise(*this);
    return Task<T>(Handle(handle));
  }

  void return_value(T value) {
    final_value = value;
    if (next) {
      next.resume();
    }
  }

  T ReturnOrThrow() {
    MaybeRethrow();
    return std::move(final_value);
  }
};

template <>
class Task<void> : public TaskBase {
  struct Promise;

 public:
  using promise_type = Promise;

  using TaskBase::TaskBase;
  using TaskBase::operator=;

  // Support explicit conversion from a value-returning Task.
  template <typename T>
    requires(!std::is_void_v<T>)
  explicit Task(Task<T> task) {
    *this = [](auto&& task) -> Task<> {
      [[maybe_unused]] auto&& val = co_await task;
    }(std::move(task));
  }

  void Wait();

  auto operator co_await() { return Awaiter<Promise>{this}; }
};

struct Task<void>::Promise : public TaskBase::PromiseBase {
  Task<void> get_return_object() {
    auto handle = std::coroutine_handle<Promise>::from_promise(*this);
    return Task<void>(Handle(handle));
  }

  void return_void() {
    if (next) {
      next.resume();
    }
  }

  void ReturnOrThrow() { MaybeRethrow(); }
};

void Task<void>::Wait() { TaskBase::template Wait<Promise>(); }
