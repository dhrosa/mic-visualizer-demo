cmake_minimum_required(VERSION 3.18)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-O2)
project(host_gui)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
add_library(image_viewer STATIC
  gui/image_viewer.h
  gui/image_viewer.cpp
)
target_link_libraries(image_viewer PRIVATE Qt6::Widgets)
target_include_directories(image_viewer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(main_window STATIC
  gui/main_window.h
  gui/main_window.cpp
)
target_link_libraries(main_window PRIVATE Qt6::Widgets image_viewer)
target_include_directories(main_window INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


add_executable(main
  main.cpp
)
target_link_libraries(main PRIVATE Qt6::Widgets main_window)

find_package(Python 3.10 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(lut lut.cpp)
target_compile_options(lut PRIVATE -march=native -O3 -g -fopenmp=libomp)
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
