cmake_minimum_required(VERSION 3.18)
project(diy C CXX)

add_library(test_main STATIC test_main.cc)
target_link_libraries(test_main gtest absl::failure_signal_handler)
function(diy_add_test target_name)
  add_executable("${target_name}" "${ARGN}")
  target_link_libraries("${target_name}" gmock gtest test_main)
  gtest_discover_tests("${target_name}" DISCOVERY_MODE PRE_TEST)
endfunction()

add_library(generator INTERFACE generator.h)
target_include_directories(generator INTERFACE .)
target_link_libraries(generator INTERFACE absl::cleanup)

diy_add_test(generator_test generator_test.cc)
target_link_libraries(generator_test generator)

add_library(buffer INTERFACE buffer.h)
target_link_libraries(buffer INTERFACE absl::any_invocable)
diy_add_test(buffer_test buffer_test.cc)
target_link_libraries(buffer_test buffer)
