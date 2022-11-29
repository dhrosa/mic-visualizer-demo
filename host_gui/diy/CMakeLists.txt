cmake_minimum_required(VERSION 3.18)
project(diy C CXX)

add_library(test_main test_main.cc)

target_link_libraries(test_main gtest absl::failure_signal_handler)
