cmake_minimum_required(VERSION 3.18)
project(audio CXX)

add_custom_command(
  OUTPUT cardinal.pcm
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/cardinal.pcm
          ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  OUTPUT cardinal.o
  COMMAND ld --relocatable --format=binary -z noexecstack --output=cardinal.o
          cardinal.pcm
  DEPENDS cardinal.pcm)

add_library(cardinal STATIC cardinal.o)

set_source_files_properties(cardinal.o PROPERTIES EXTERNAL_OBJECT true GENERATED
                                                                       true)
set_target_properties(cardinal PROPERTIES LINKER_LANGUAGE C)

add_library(audio_source source.cc source.h)

target_link_libraries(audio_source cardinal xtensor generator absl::time)

add_executable(audio_source_test source_test.cc)
target_link_libraries(audio_source_test audio_source gmock gtest
                      GTest::gtest_main absl::failure_signal_handler)
gtest_discover_tests(audio_source_test)
