cmake_minimum_required(VERSION 3.18)
project(diy C CXX)

add_library(test_main STATIC test_main.cc)
target_link_libraries(test_main gtest absl::failure_signal_handler)
function(diy_cc_test name)
  set(options "")
  set(single_value_args "")
  set(multi_value_args SOURCES LIBRARIES)
  cmake_parse_arguments("" "${options}" "${single_value_args}"
                        "${multi_value_args}" ${ARGN})
  add_executable(${name} "${_SOURCES}")
  target_link_libraries(${name} gmock gtest test_main "${_LIBRARIES}")
  gtest_discover_tests(${name} DISCOVERY_MODE PRE_TEST)
endfunction()

function(diy_cc_library name)
  set(options STATIC)
  set(single_value_args "")
  set(multi_value_args HEADERS SOURCES LIBRARIES)
  cmake_parse_arguments("" "${options}" "${single_value_args}"
                        "${multi_value_args}" ${ARGN})
  set(library_type "")
  if(_SRCS STREQUAL "")
    set(library_type INTERFACE)
  elseif(_STATIC)
    set(library_type STATIC)
  endif()
  add_library(${name} ${library_type})
  target_sources(${name} PRIVATE ${_HEADERS} ${_SOURCES})
  set_property(TARGET ${name} PROPERTY LINKER_LANGUAGE CXX)
  target_link_libraries(${name} ${_LIBRARIES})
endfunction()

diy_cc_library(
  generator
  HEADERS generator.h
  LIBRARIES absl::cleanup)

diy_cc_test(
  generator_test
  SOURCES generator_test.cc
  LIBRARIES generator)

diy_cc_library(
  buffer
  HEADERS buffer.h
  LIBRARIES absl::any_invocable)
diy_cc_test(
  buffer_test
  SOURCES buffer_test.cc
  LIBRARIES buffer)
