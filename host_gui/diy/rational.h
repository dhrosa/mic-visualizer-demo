#pragma once

#include <concepts>
#include <cstdint>
#include <iostream>

// Simple rational number implementation, just supporting a few operations used
// in this project.
struct Rational {
  std::int64_t numerator;
  std::int64_t denominator;

  explicit operator double() {
    return static_cast<double>(numerator) / denominator;
  }

  Rational operator*(std::integral auto scale) const {
    return {numerator * scale, denominator};
  }

  bool operator==(Rational other) const {
    return numerator == other.numerator && denominator == other.denominator;
  }

  Rational reciprocal() const { return {denominator, numerator}; }
};

inline Rational operator*(std::integral auto scale, Rational r) {
  return r * scale;
}

inline std::ostream& operator<<(std::ostream& s, Rational r) {
  return s << r.numerator << "/" << r.denominator;
}
