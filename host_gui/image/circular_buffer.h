#pragma once

#include <Eigen/Core>
#include <ranges>
#include <span>

// Models a 2D image where new columns are appended to the right, causing older
// columsn to be dropped from the left. To support this operation efficiently,
// we don't expose the entire underlying 2D array to the caller, but instead
// expose Older() and a Newer() 2D array accessors, which when concatenated
// horizontally form the full image.
template <typename T>
class CircularBuffer {
 public:
  CircularBuffer(std::size_t width, std::size_t height, T fill = T{})
      : data_(height, width), next_column_(width) {
    data_.fill(fill);
  }

  auto Newer() const noexcept { return data_.leftCols(next_column_); }

  auto Older() const noexcept {
    return data_.rightCols(columns() - next_column_);
  }

  void AppendColumn(std::span<const T> column) noexcept;

  std::size_t columns() const noexcept { return data_.cols(); }
  std::size_t rows() const noexcept { return data_.rows(); }

  std::size_t width() const noexcept { return columns(); }
  std::size_t height() const noexcept { return rows(); }

 private:
  Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic> data_;
  Eigen::Index next_column_ = 0;
};

template <typename T>
void CircularBuffer<T>::AppendColumn(std::span<const T> column) noexcept {
  next_column_ %= columns();
  std::ranges::copy(column, data_.col(next_column_).begin());
  ++next_column_;
}
