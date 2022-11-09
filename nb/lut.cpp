#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <cstdint>
#include <stdexcept>
#include <cstring>

using uint8 = std::uint8_t;
using uint32 = std::uint32_t;

struct Table {
  std::vector<uint32> entries;

  static Table FromRgbaArray(pybind11::array_t<uint8> arr);
};

Table Table::FromRgbaArray(pybind11::array_t<uint8> arr) {
  if (arr.size() == 0) {
    return Table();
  }
  if (arr.ndim() != 2) {
    throw std::runtime_error("Array must be 2D: " + std::to_string(arr.ndim()));
  }
  if (arr.shape(1) != 4) {
    throw std::runtime_error("Innermost dimension must be 4: " + std::to_string(arr.shape(0)));
  }
  const pybind11::ssize_t rows = arr.shape(0);
  const auto view = arr.unchecked<2>();
  Table table;
  table.entries.resize(rows);
  uint32* output = table.entries.data();
  for (pybind11::ssize_t row = 0; row < rows; ++row) {
    const uint8
      r = view(row, 0),
      g = view(row, 1),
      b = view(row, 2),
      a = view(row, 3);
    const uint8 values[] = {b, g, r, a};
    std::memcpy(output + row,
                values,
                4);
    
  }
  return table;
}


PYBIND11_MODULE(lut, m) {
  pybind11::class_<Table>(m, "Table")
    .def(pybind11::init(&Table::FromRgbaArray))
    .def_readonly("entries", &Table::entries);
}

/*
<%
setup_pybind11(cfg)
%>
*/
