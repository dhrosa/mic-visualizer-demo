cmake_minimum_required(VERSION 3.18)
project(diy C CXX)

add_library(test_main test_main.cc)
target_link_libraries(test_main gtest absl::failure_signal_handler)

add_library(generator INTERFACE generator.h)
target_include_directories(generator INTERFACE .)
target_link_libraries(generator INTERFACE absl::cleanup)

add_executable(generator_test generator_test.cc)
target_link_libraries(generator_test generator gmock gtest test_main)
gtest_discover_tests(generator_test)
