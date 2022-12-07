cmake_minimum_required(VERSION 3.18)
project(gui CXX)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/colormaps.gen.cc
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/generate_colormaps.py >
          ${CMAKE_CURRENT_BINARY_DIR}/colormaps.gen.cc
  DEPENDS generate_colormaps.py
  VERBATIM)

diy_cc_library(colormaps AUTO SOURCES colormaps.gen.cc)
target_include_directories(colormaps PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                             ${CMAKE_CURRENT_BINARY_DIR})

diy_cc_library(cursor AUTO LIBRARIES Qt6::Widgets)

diy_cc_library(
  image_viewer AUTO LIBRARIES Qt6::Widgets cursor absl::function_ref
                              absl::synchronization)
set_property(TARGET image_viewer PROPERTY AUTOMOC ON)

diy_cc_library(colormap_picker AUTO LIBRARIES Qt6::Widgets colormaps)

diy_cc_library(
  main_window AUTO
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
