#pragma once

#include <absl/functional/any_invocable.h>

#include <ranges>
#include <span>


// An std::contiguous_range modeling a std::span with an RAII cleanup function
// that takes care of cleaning the data backing the span.
//
// This allows for cheaply passing around contiguous data that can be
// backed in versatile ways (e.g. by an std::vector, allocated with
// special alignment, etc...).
template <typename T>
class Buffer : public std::ranges::view_interface<Buffer<T>> {
 public:
  using value_type = std::span<T>::value_type;

  Buffer() noexcept = default;
  Buffer(std::span<T> span, absl::AnyInvocable<void() &&> cleanup) noexcept
      : span_(span), cleanup_(std::move(cleanup)) {}

  // Convenience factory that creates a buffer of the given size with
  // uninitialized contents.
  static Buffer<T> Uninitialized(std::size_t n) noexcept;

  ~Buffer() {
    if (cleanup_) {
      std::move(cleanup_)();
    }
  }

  Buffer(Buffer&&) noexcept = default;
  Buffer& operator=(Buffer&&) noexcept = default;

  std::span<T> span() const noexcept { return span_; }
  auto begin() const noexcept { return span_.begin(); }
  auto end() const noexcept { return span_.end(); }
  
 private:
  std::span<T> span_;
  absl::AnyInvocable<void() &&> cleanup_;
};

template <typename T>
Buffer<T> Buffer<T>::Uninitialized(std::size_t n) noexcept {
  auto storage = std::make_unique_for_overwrite<T[]>(n);
  std::span<T> span(storage.get(), n);
  return Buffer(span, [storage = std::move(storage)] {});
}
