cmake_minimum_required(VERSION 3.18)
project(image CXX)

diy_cc_library(circular_buffer HEADERS circular_buffer.h)
target_include_directories(circular_buffer INTERFACE ${EIGEN_SOURCE_DIR})

diy_cc_test(circular_buffer_test AUTO)

diy_cc_library(
  qimage_eigen
  HEADERS qimage_eigen.h
  LIBRARIES Qt6::Gui)
target_include_directories(qimage_eigen INTERFACE ${EIGEN_SOURCE_DIR})

diy_cc_test(qimage_eigen_test AUTO LIBRARIES Qt6::Gui)
