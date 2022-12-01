cmake_minimum_required(VERSION 3.18)
project(gui CXX)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/colormaps.gen.cc
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/generate_colormaps.py >
          ${CMAKE_CURRENT_BINARY_DIR}/colormaps.gen.cc
  DEPENDS generate_colormaps.py
  VERBATIM)

diy_cc_library(
  NAME
  colormaps
  STATIC
  HEADERS
  colormaps.h
  SOURCES
  colormaps.cc
  colormaps.gen.cc)
target_include_directories(colormaps PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                             ${CMAKE_CURRENT_BINARY_DIR})

diy_cc_library(
  NAME
  image_viewer
  STATIC
  HEADERS
  image_viewer.h
  SOURCES
  image_viewer.cc
  LIBRARIES
  Qt6::Widgets
  colormaps)

diy_cc_library(
  NAME
  colormap_picker
  STATIC
  HEADERS
  colormap_picker.h
  SOURCES
  colormap_picker.cc
  LIBRARIES
  Qt6::Widgets
  colormaps)

diy_cc_library(
  NAME
  main_window
  STATIC
  HEADERS
  main_window.h
  SOURCES
  main_window.cc
  LIBRARIES
  Qt6::Widgets
  colormaps
  image_viewer
  colormap_picker)
set_property(TARGET main_window PROPERTY AUTOMOC ON)
