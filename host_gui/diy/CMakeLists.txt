cmake_minimum_required(VERSION 3.18)
project(diy C CXX)

add_library(test_main STATIC test_main.cc)
target_link_libraries(test_main gtest absl::failure_signal_handler)
function(diy_add_test target_name)
  add_executable("${target_name}" "${ARGN}")
  target_link_libraries("${target_name}" gmock gtest test_main)
  gtest_discover_tests("${target_name}" DISCOVERY_MODE PRE_TEST)
endfunction()

function(diy_cc_library)
  set(options STATIC)
  set(single_value_args NAME)
  set(multi_value_args HEADERS SOURCES LIBRARIES)
  cmake_parse_arguments(DIY_CC_LIB "${options}" "${single_value_args}"
                        "${multi_value_args}" ${ARGN})
  set(library_type "")
  set(name ${DIY_CC_LIB_NAME})
  if(DIY_CC_LIB_SRCS STREQUAL "")
    set(library_type INTERFACE)
  elseif(DIY_CC_LIB_STATIC)
    set(library_type STATIC)
  endif()
  add_library(${name} ${library_type})
  target_sources(${name} PRIVATE ${DIY_CC_LIB_HEADERS} ${DIY_CC_LIB_SOURCES})
  set_property(TARGET ${name} PROPERTY LINKER_LANGUAGE CXX)
  target_link_libraries(${name} ${DIY_CC_LIB_LIBRARIES})
endfunction()

diy_cc_library(
  NAME generator
  HEADERS generator.h
  LIBRARIES absl::cleanup)

diy_add_test(generator_test generator_test.cc)
target_link_libraries(generator_test generator)

diy_cc_library(
  NAME buffer
  HEADERS buffer.h
  LIBRARIES absl::any_invocable)
diy_add_test(buffer_test buffer_test.cc)
target_link_libraries(buffer_test buffer)
