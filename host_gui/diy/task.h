#pragma once

#include <concepts>
#include <coroutine>
#include <exception>
#include <latch>
#include <type_traits>
#include <utility>

#include "handle.h"

template <typename T = void>
class Task {
  struct Promise;

  template <typename F, typename... Args>
  using MapResult = std::invoke_result_t<F, T, Args...>;

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
      : Task(std::move(other).Map([](auto&&) {})) {}

  bool done() const noexcept { return handle_->done(); }

  auto operator co_await();

  T Wait();

  // Creates a new Task whose value is the result of applying `f` to
  // the retuen value of the current task.
  template <typename F, typename... Args>
  Task<MapResult<F, Args...>> Map(F&& f, Args&&... args) &&;

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
  void return_value(U&& value) {
    this->final_value = std::move(value);
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
  auto final_suspend() noexcept {
    struct FinalAwaiter {
      std::coroutine_handle<> parent;

      bool await_ready() noexcept { return false; }

      std::coroutine_handle<> await_suspend(
          [[maybe_unused]] std::coroutine_handle<> task_handle) noexcept {
        if (parent) {
          return parent;
        }
        return std::noop_coroutine();
      }

      void await_resume() noexcept {}
    };
    return FinalAwaiter{std::exchange(parent, nullptr)};
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
  // We create a custom coroutine that synchronously notifies the caller when
  // its complete.
  struct SyncPromise;

  struct SyncTask {
    Handle handle;

    using promise_type = SyncPromise;
  };

  // TODO(dhrosa): This is largely similar to Task::Promise.
  struct SyncPromise
      : std::conditional_t<kIsVoidTask, VoidPromiseBase, ValuePromiseBase> {
    std::latch complete{1};
    std::exception_ptr exception;

    SyncTask get_return_object() {
      return {Handle(std::coroutine_handle<SyncPromise>::from_promise(*this))};
    }

    void unhandled_exception() { exception = std::current_exception(); }
    auto initial_suspend() { return std::suspend_never{}; }
    auto final_suspend() noexcept {
      complete.count_down();
      return std::suspend_always{};
    }

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

  auto sync_task = [](Task<T>& task) -> SyncTask {
    co_return (co_await task);
  }(*this);
  SyncPromise& promise = sync_task.handle.template promise<SyncPromise>();
  promise.complete.wait();
  return promise.ReturnOrThrow();
}

template <typename T>
template <typename F, typename... Args>
auto Task<T>::Map(F&& f, Args&&... args) && -> Task<MapResult<F, Args...>> {
  return [](Task<T> task, F f, Args... args) -> Task<MapResult<F, Args...>> {
    co_return f(co_await task, args...);
  }(std::move(*this), std::move(f), std::move(args)...);
}
