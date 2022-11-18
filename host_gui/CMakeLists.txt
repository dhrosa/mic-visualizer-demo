cmake_minimum_required(VERSION 3.18)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-O2)
project(host_gui CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(generator INTERFACE
  generator.h
)
target_include_directories(generator INTERFACE .)

add_custom_command(
  OUTPUT "gui/colormaps.gen.cc"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMAND ./generate_luts.py > gui/colormaps.gen.cc
  DEPENDS generate_luts.py
  VERBATIM
)

add_library(colormaps STATIC
  gui/colormaps.h
  gui/colormaps.cc
  gui/colormaps.gen.cc
)

add_library(image_viewer STATIC
  gui/image_viewer.h
  gui/image_viewer.cpp
)
target_link_libraries(image_viewer PRIVATE Qt6::Widgets colormaps)

add_library(colormap_picker STATIC
  gui/colormap_picker.h
  gui/colormap_picker.cc
)
target_link_libraries(colormap_picker PRIVATE Qt6::Widgets colormaps)

add_library(main_window STATIC
  gui/main_window.h
  gui/main_window.cpp
)
target_link_libraries(main_window PRIVATE Qt6::Widgets generator colormaps image_viewer colormap_picker)
set_property(TARGET main_window PROPERTY AUTOMOC ON)


add_executable(main
  main.cpp
)
target_link_libraries(main PRIVATE Qt6::Widgets main_window)

file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
  ${PROJECT_SOURCE_DIR}/*.[ch]pp
)
add_custom_target(
  format_cc
  COMMAND clang-format -i ${ALL_CXX_SOURCE_FILES}
  VERBATIM
)
