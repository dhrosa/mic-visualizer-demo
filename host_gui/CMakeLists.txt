cmake_minimum_required(VERSION 3.18)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-O2)
project(host_gui CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)
enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(generator INTERFACE generator.h)
target_include_directories(generator INTERFACE .)

add_executable(generator_test generator_test.cc)
target_link_libraries(generator_test generator gmock GTest::gtest_main)
gtest_discover_tests(generator_test)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/colormaps.gen.cc
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/generate_luts.py >
          ${CMAKE_CURRENT_BINARY_DIR}/colormaps.gen.cc
  DEPENDS generate_luts.py
  VERBATIM)

add_library(colormaps STATIC gui/colormaps.h gui/colormaps.cc
                             ${CMAKE_CURRENT_BINARY_DIR}/colormaps.gen.cc)
target_include_directories(colormaps PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gui
                                             ${CMAKE_CURRENT_BINARY_DIR})

add_library(image_viewer STATIC gui/image_viewer.h gui/image_viewer.cc)
target_link_libraries(image_viewer PRIVATE Qt6::Widgets colormaps)

add_library(colormap_picker STATIC gui/colormap_picker.h gui/colormap_picker.cc)
target_link_libraries(colormap_picker PRIVATE Qt6::Widgets colormaps)

add_library(main_window STATIC gui/main_window.h gui/main_window.cc)
target_link_libraries(main_window PRIVATE Qt6::Widgets colormaps image_viewer
                                          colormap_picker)
set_property(TARGET main_window PROPERTY AUTOMOC ON)

add_executable(main main.cc)
target_link_libraries(main PRIVATE Qt6::Widgets main_window)

add_custom_target(
  format_cc
  COMMAND find ${PROJECT_SOURCE_DIR} -type f | grep -v ${PROJECT_BINARY_DIR} |
          grep -E "(cc|h)$" | xargs -L1 clang-format -i
  VERBATIM)
add_custom_target(
  format_cmake
  COMMAND cmake-format -i ${CMAKE_CURRENT_LIST_FILE}
  VERBATIM)
add_custom_target(format DEPENDS format_cc format_cmake)

add_custom_target(
  itest COMMAND find ${PROJECT_SOURCE_DIR} -type f | grep -v
                ${PROJECT_BINARY_DIR} | entr -r sh -c 'make -j && ctest')
