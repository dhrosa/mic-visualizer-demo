#pragma once

#include <coroutine>
#include <exception>
#include <type_traits>
#include <utility>

#include "handle.h"

// TODO(dhrosa): The inheritance going on might be easier if we instead
// conditionally inherit from a base type whose value depends on whether T is
// void.

class TaskBase {
 public:
  TaskBase() = default;
  TaskBase(Handle handle) noexcept : handle_(std::move(handle)) {}
  TaskBase(TaskBase&& other) noexcept = default;
  TaskBase& operator=(TaskBase&& other) noexcept = default;
  ~TaskBase() = default;

  bool done() const noexcept { return handle_->done(); }

 protected:
  struct PromiseBase;

  // Awaitable for awaiting the completion of this task.
  template <typename P>
  struct Awaiter;

  Handle handle_;
};

struct TaskBase::PromiseBase {
  // The exception thrown by body of the task, if any.
  std::exception_ptr exception;

  // The suspended coroutine awaiting this task's completion, if any.
  std::coroutine_handle<> parent;

  // Lazy execution. Task body is deferred to the first explicit resume() call.
  std::suspend_always initial_suspend() noexcept { return {}; }

  // Resume execution of parent coroutine that was awaiting this task's
  // completion, if any.
  std::suspend_always final_suspend() noexcept {
    if (parent) {
      parent.resume();
    }
    return {};
  }

  void unhandled_exception() { exception = std::current_exception(); }

  void MaybeRethrow() {
    if (exception) {
      std::rethrow_exception(exception);
    }
  }
};

template <typename P>
struct TaskBase::Awaiter {
  // The child task whose completion is being awaited.
  TaskBase* task;

  // Always suspend the parent.
  bool await_ready() const noexcept { return false; }

  // Tell the child task to resume the parent (current task) when it completes.
  // Then context switch into the child task.
  std::coroutine_handle<> await_suspend(std::coroutine_handle<> parent) {
    task->handle_.template promise<P>().parent = parent;
    return task->handle_.get();
  }

  // Child task has completed; return its final value.
  auto await_resume() {
    return task->handle_.template promise<P>().ReturnOrThrow();
  }
};

template <typename T = void>
class Task : public TaskBase {
  struct Promise;

 public:
  using promise_type = Promise;

  using TaskBase::TaskBase;
  using TaskBase::operator=;

  auto operator co_await() { return Awaiter<Promise>{this}; }

  T Wait();
};

template <typename T>
T Task<T>::Wait() {
  // An arbitrary task can suspend any number of times, so simply resuming the
  // task doesn't mean it will then be complete. So we contruct a trivial
  // `notifier` coroutine that has known suspension points; the initial
  // suspension point (since Promise::initial_suspend() always suspends), and
  // the awaiting of task completion (Awaiter always suspends the parent
  // coroutine)
  //
  // After two resume() calls for the above suspension points, control won't
  // return back to us until the end of `notifier`'s body, at which point we
  // know that the waited on task has completed.
  auto notifier = [](Task<T>& task) -> Task<T> {
    co_return (co_await task);
  }(*this);
  notifier.handle_->resume();
  notifier.handle_->resume();
  return notifier.handle_.template promise<Promise>().ReturnOrThrow();
}

template <typename T>
struct Task<T>::Promise : public TaskBase::PromiseBase {
  using value_type = T;

  T final_value;

  Task<T> get_return_object() {
    auto handle = std::coroutine_handle<Promise>::from_promise(*this);
    return Task<T>(Handle(handle));
  }

  void return_value(T value) { final_value = value; }

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

  auto operator co_await() { return Awaiter<Promise>{this}; }

  void Wait();
};

struct Task<void>::Promise : public TaskBase::PromiseBase {
  using value_type = void;

  Task<void> get_return_object() {
    auto handle = std::coroutine_handle<Promise>::from_promise(*this);
    return Task<void>(Handle(handle));
  }

  void return_void() {}

  void ReturnOrThrow() { MaybeRethrow(); }
};

void Task<void>::Wait() {
  // An arbitrary task can suspend any number of times, so simply resuming the
  // task doesn't mean it will then be complete. So we contruct a trivial
  // `notifier` coroutine that has known suspension points; the initial
  // suspension point (since Promise::initial_suspend() always suspends), and
  // the awaiting of task completion (Awaiter always suspends the parent
  // coroutine)
  //
  // After two resume() calls for the above suspension points, control won't
  // return back to us until the end of `notifier`'s body, at which point we
  // know that the waited on task has completed.
  auto notifier = [](Task<>& task) -> Task<> { co_await task; }(*this);
  notifier.handle_->resume();
  notifier.handle_->resume();
  notifier.handle_.template promise<Promise>().ReturnOrThrow();
}
