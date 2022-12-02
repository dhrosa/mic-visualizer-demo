cmake_minimum_required(VERSION 3.18)
project(image CXX)

diy_cc_library(circular_buffer HEADERS circular_buffer.h)
target_include_directories(circular_buffer INTERFACE ${EIGEN_SOURCE_DIR})

diy_add_test(
  circular_buffer_test
  SOURCES circular_buffer_test.cc
  LIBRARIES circular_buffer)
