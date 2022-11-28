cmake_minimum_required(VERSION 3.18)
project(gui CXX)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/colormaps.gen.cc
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/generate_colormaps.py >
          ${CMAKE_CURRENT_BINARY_DIR}/colormaps.gen.cc
  DEPENDS generate_colormaps.py
  VERBATIM)

add_library(colormaps STATIC colormaps.h colormaps.cc colormaps.gen.cc)
target_include_directories(colormaps PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                             ${CMAKE_CURRENT_BINARY_DIR})

add_library(image_viewer STATIC image_viewer.h image_viewer.cc)
target_link_libraries(image_viewer PRIVATE Qt6::Widgets colormaps)

add_library(colormap_picker STATIC colormap_picker.h colormap_picker.cc)
target_link_libraries(colormap_picker PRIVATE Qt6::Widgets colormaps)

add_library(main_window STATIC main_window.h main_window.cc)
target_link_libraries(main_window PRIVATE Qt6::Widgets colormaps image_viewer
                                          colormap_picker)
set_property(TARGET main_window PROPERTY AUTOMOC ON)
