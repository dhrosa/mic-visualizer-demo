cmake_minimum_required(VERSION 3.18)
project(image CXX)

diy_cc_library(circular_buffer HEADERS circular_buffer.h)
target_include_directories(circular_buffer INTERFACE ${EIGEN_SOURCE_DIR})

diy_cc_test(
  circular_buffer_test
  SOURCES circular_buffer_test.cc
  LIBRARIES circular_buffer)

diy_cc_library(
  qimage_eigen
  HEADERS qimage_eigen.h
  LIBRARIES Qt6::Gui)
target_include_directories(qimage_eigen INTERFACE ${EIGEN_SOURCE_DIR})

diy_cc_test(
  qimage_eigen_test
  SOURCES qimage_eigen_test.cc
  LIBRARIES qimage_eigen Qt6::Gui)
