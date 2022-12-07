cmake_minimum_required(VERSION 3.18)
project(image CXX)

diy_cc_library(circular_buffer AUTO LIBRARIES eigen)
diy_cc_test(circular_buffer_test AUTO)

diy_cc_library(qimage_eigen AUTO LIBRARIES Qt6::Gui eigen)
diy_cc_test(qimage_eigen_test AUTO LIBRARIES Qt6::Gui)

diy_cc_library(lut AUTO STATIC LIBRARIES eigen)
if(MIC_ENABLE_OPENMP)
  target_link_libraries(lut OpenMP::OpenMP_CXX)
endif()

diy_cc_test(lut_test AUTO eigen)

diy_cc_binary(
  lut_benchmark AUTO LIBRARIES lut Qt6::Gui eigen benchmark::benchmark
                               benchmark::benchmark_main)
