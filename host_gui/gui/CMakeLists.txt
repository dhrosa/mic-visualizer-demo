cmake_minimum_required(VERSION 3.18)
project(gui CXX)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/colormaps.gen.cc
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/generate_colormaps.py >
          ${CMAKE_CURRENT_BINARY_DIR}/colormaps.gen.cc
  DEPENDS generate_colormaps.py
  VERBATIM)

diy_cc_library(
  colormaps
  HEADERS colormaps.h
  SOURCES colormaps.cc colormaps.gen.cc)
target_include_directories(colormaps PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                             ${CMAKE_CURRENT_BINARY_DIR})

diy_cc_library(
  image_viewer
  HEADERS image_viewer.h
  SOURCES image_viewer.cc
  LIBRARIES Qt6::Widgets colormaps)

diy_cc_library(
  colormap_picker
  HEADERS colormap_picker.h
  SOURCES colormap_picker.cc
  LIBRARIES Qt6::Widgets colormaps)

diy_cc_library(
  main_window
  HEADERS main_window.h
  SOURCES main_window.cc
  LIBRARIES Qt6::Widgets
            absl::synchronization
            absl::log
            absl::time
            source
            spectrum
            colormaps
            image_viewer
            colormap_picker
            generator
            latency_logger
            circular_buffer
            qimage_eigen
            buffer)
set_property(TARGET main_window PROPERTY AUTOMOC ON)
