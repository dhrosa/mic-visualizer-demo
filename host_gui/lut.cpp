#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#include <omp.h>

using uint8 = std::uint8_t;
using uint32 = std::uint32_t;

struct Table {
  std::vector<uint32> entries;

  static Table FromRgbaArray(pybind11::array_t<uint8> arr);

  void Map(double vmin, double vmax, pybind11::array_t<double> source,
           pybind11::array_t<uint32> dest);
};

Table FromRgbaArray(pybind11::array_t<uint8> arr) {
  if (arr.size() == 0) {
    throw std::invalid_argument("Input is empty.");
  }
  if (arr.ndim() != 2) {
    throw std::invalid_argument("Array must be 2D: " +
                                std::to_string(arr.ndim()));
  }
  if (arr.shape(1) != 4) {
    throw std::invalid_argument("Innermost dimension must be 4: " +
                                std::to_string(arr.shape(1)));
  }
  const pybind11::ssize_t rows = arr.shape(0);
  const auto view = arr.unchecked<2>();
  Table table;
  table.entries.resize(rows);
  uint32 *output = table.entries.data();
  for (pybind11::ssize_t row = 0; row < rows; ++row) {
    const uint8 r = view(row, 0), g = view(row, 1), b = view(row, 2),
                a = view(row, 3);
    const uint8 values[] = {b, g, r, a};
    std::memcpy(output + row, values, 4);
  }
  return table;
}

inline void MapRow(double vmin, double vmax, const uint32 *__restrict lut,
                   pybind11::ssize_t lut_size, const double *source,
                   uint32 *__restrict dest, pybind11::ssize_t width) {
  const double vrange = vmax - vmin;
#pragma clang loop unroll(enable) vectorize_width(32) interleave_count(8)
  for (pybind11::ssize_t col = 0; col < width; ++col) {
    const double clamped = std::clamp(source[col], vmin, vmax);
    const double unbiased = clamped - vmin;
    const double normalized = unbiased / vrange;
    const pybind11::ssize_t index = normalized * (lut_size - 1);
    dest[col] = lut[index];
  }
}

void Table::Map(double vmin, double vmax, pybind11::array_t<double> source,
                pybind11::array_t<uint32> dest) {
  if (source.ndim() != 2) {
    throw std::runtime_error("Source must be 2D: " +
                             std::to_string(source.ndim()));
  }
  if (dest.ndim() != 2) {
    throw std::runtime_error("Destination must be 2D: " +
                             std::to_string(dest.ndim()));
  }
  for (int axis : {0, 1}) {
    const int s = source.shape(axis);
    const int d = dest.shape(axis);
    if (s != d) {
      throw std::runtime_error(
          "Source and destination have mismatched shapes: " +
          std::to_string(s) + " vs " + std::to_string(d));
    }
  }
  const pybind11::ssize_t height = source.shape(0);
  const pybind11::ssize_t width = source.shape(1);
  const pybind11::ssize_t n = entries.size();
  if (n == 0) {
    return;
  }
  const auto source_view = source.unchecked<2>();
  auto dest_view = dest.mutable_unchecked<2>();
  const uint32 *lut = entries.data();
  pybind11::gil_scoped_release gil_release;
#pragma omp parallel for schedule(nonmonotonic : guided)
  for (pybind11::ssize_t row = 0; row < height; ++row) {
    MapRow(vmin, vmax, lut, n, &source_view(row, 0), &dest_view(row, 0), width);
  }
}

PYBIND11_MODULE(lut, m) {
  pybind11::class_<Table>(m, "Table")
      .def(pybind11::init(&FromRgbaArray))
      .def_readonly("entries", &Table::entries)
      .def("Map", &Table::Map, pybind11::arg("vmin"), pybind11::arg("vmax"),
           pybind11::arg("source").noconvert(),
           pybind11::arg("dest").noconvert());
}
