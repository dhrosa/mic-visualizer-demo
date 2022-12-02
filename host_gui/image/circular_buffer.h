#pragma once

#include <Eigen/Core>
#include <span>

template <typename T>
class CircularBuffer {
 public:
  CircularBuffer(std::size_t width, std::size_t height, T fill = T{})
      : data_(height, width) {
    data_.fill(fill);
  }

  auto Newer() const noexcept { return data_.leftCols(current_column_); }

  auto Older() const noexcept {
    return data_.rightCols(columns() - current_column_);
  }

  void AppendColumn(std::span<const T> new_column);

  std::size_t columns() const noexcept { return data_.cols(); }
  std::size_t rows() const noexcept { return data_.rows(); }

  std::size_t width() const noexcept { return columns(); }
  std::size_t height() const noexcept { return rows(); }

 private:
  Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic> data_;
  Eigen::Index current_column_ = 0;
};

template <typename T>
void CircularBuffer<T>::AppendColumn(std::span<const T> new_column) {}
