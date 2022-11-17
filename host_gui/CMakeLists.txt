cmake_minimum_required(VERSION 3.18)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
project(host_gui)

find_package(Python 3.10 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(lut lut.cpp)
target_compile_options(lut PRIVATE -std=c++20 -march=native -O3 -g -fopenmp=libomp)
target_link_libraries(lut PRIVATE iomp5)

add_custom_target(tests
  COMMAND python3 -m unittest discover .
  VERBATIM
)

add_custom_command(
  OUTPUT profile_data
  COMMAND py-spy record --native --duration 3 --format speedscope -o profile_data -- ./main.py
  DEPENDS lut
)

add_custom_target(
  profile
  COMMAND node node_modules/speedscope/bin/cli.js profile_data
  DEPENDS profile_data
)
