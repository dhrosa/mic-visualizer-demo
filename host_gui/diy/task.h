#pragma once

#include <concepts>
#include <coroutine>
#include <exception>
#include <type_traits>
#include <utility>

#include "handle.h"

template <typename T = void>
class Task {
  struct Promise;

 public:
  using promise_type = Promise;

  Task() = default;
  Task(Task&& other) noexcept = default;
  Task& operator=(Task&& other) noexcept = default;
  ~Task() = default;

  // Allow implicit converson of Task<T> to Task<>.
  template <typename U>
  Task(Task<U> other)
    requires(std::same_as<T, void> && !std::same_as<U, void>)
  {
    *this = [](Task<U> task) -> Task<> {
      [[maybe_unused]] U value = co_await task;
    }(std::move(other));
  };

  bool done() const noexcept { return handle_->done(); }

  auto operator co_await();

  T Wait();

 private:
  static constexpr bool kIsVoidTask = std::same_as<T, void>;

  struct VoidPromiseBase;
  struct ValuePromiseBase;
  struct Awaiter;

  Promise& promise() { return handle_.template promise<Promise>(); }

  Handle handle_;
};

template <typename T>
struct Task<T>::VoidPromiseBase {
  void return_void() {}
};

template <typename T>
struct Task<T>::ValuePromiseBase {
  T final_value;

  template <typename U>
  void return_value(U&& value)
    requires(!kIsVoidTask)
  {
    this->final_value = std::forward<U>(value);
  }
};

template <typename T>
struct Task<T>::Promise
    : std::conditional_t<kIsVoidTask, VoidPromiseBase, ValuePromiseBase> {
  // The exception thrown by body of the task, if any.
  std::exception_ptr exception;

  // The suspended coroutine awaiting this task's completion, if any.
  std::coroutine_handle<> parent;

  Task<T> get_return_object() {
    Task<T> task;
    task.handle_ = Handle(std::coroutine_handle<Promise>::from_promise(*this));
    return task;
  }

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

  T ReturnOrThrow() {
    if (exception) {
      std::rethrow_exception(exception);
    }
    if constexpr (kIsVoidTask) {
      return;
    } else {
      return std::move(this->final_value);
    }
  }
};

template <typename T>
struct Task<T>::Awaiter {
  // The child task whose completion is being awaited.
  Task<T>* task;

  // Always suspend the parent.
  bool await_ready() const noexcept { return false; }

  // Tell the child task to resume the parent (current task) when it completes.
  // Then context switch into the child task.
  std::coroutine_handle<> await_suspend(std::coroutine_handle<> parent) {
    task->promise().parent = parent;
    return task->handle_.get();
  }

  // Child task has completed; return its final value.
  auto await_resume() { return task->promise().ReturnOrThrow(); }
};

template <typename T>
auto Task<T>::operator co_await() {
  return Awaiter{this};
}

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
  return notifier.promise().ReturnOrThrow();
}
