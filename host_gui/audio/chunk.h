#pragma once

#include <algorithm>
#include <ranges>
#include <span>
#include <vector>

#include "diy/coro/async_generator.h"

// Given a stream of arbitrarily-sized input frames, generates a stream of
// output frames with a fixed size of `n`.
template <typename T>
AsyncGenerator<std::vector<T>> Chunked(AsyncGenerator<std::vector<T>> source,
                                       std::size_t n) {
  // Buffer holding the current output frame. Always has length `n` and has
  // arbitrary contents between output frames.
  std::vector<T> chunk(n);
  // Region of the current output frame that still needs to be filled.
  std::span<T> unfilled_chunk_span(chunk);
  // The current input frame we're copying data from.
  std::vector<T>* source_frame = co_await source;
  if (!source_frame) {
    // Source produced zero input frames.
    co_return;
  }
  // The region of the current input frame that we've yet to transfer data from
  // yet.
  std::span<T> current_source_span = *source_frame;
  while (true) {
    if (current_source_span.empty()) {
      // Current input frame is exhausted; retrieve the next one.
      source_frame = co_await source;
      if (!source_frame) {
        co_return;
      }
      current_source_span = *source_frame;
    }
    // Fill up the rest of the output frame, or consume the entire input frame;
    // whichever happens first.
    const std::size_t copy_count =
        std::min(unfilled_chunk_span.size(), current_source_span.size());
    std::ranges::copy(current_source_span.first(copy_count),
                      unfilled_chunk_span.begin());
    current_source_span = current_source_span.subspan(copy_count);
    unfilled_chunk_span = unfilled_chunk_span.subspan(copy_count);
    if (unfilled_chunk_span.empty()) {
      // No more data needed to fill the current output frame; emit it.
      co_yield chunk;
      // Mark entire output frame as needing to be filled again.
      unfilled_chunk_span = chunk;
    }
  }
}
