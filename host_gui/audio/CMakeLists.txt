cmake_minimum_required(VERSION 3.18)
project(audio C CXX)

add_custom_command(
  OUTPUT cardinal.o
  BYPRODUCTS cardinal.pcm
  DEPENDS cardinal.pcm
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/cardinal.pcm
          ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ld --relocatable --format=binary -z noexecstack --output=cardinal.o
          cardinal.pcm)

add_library(cardinal STATIC cardinal.o)

set_source_files_properties(cardinal.o PROPERTIES EXTERNAL_OBJECT true GENERATED
                                                                       true)
set_target_properties(cardinal PROPERTIES LINKER_LANGUAGE C)

diy_cc_library(chunk AUTO LIBRARIES buffer diy_coro)
diy_cc_test(chunk_test AUTO LIBRARIES diy_coro)

diy_cc_library(source AUTO LIBRARIES cardinal diy_coro buffer absl::time)

diy_cc_test(source_test AUTO)

diy_cc_library(spectrum AUTO LIBRARIES fftw3 diy_coro buffer)
# For some reason the fftw3 library itself doesn't automatically add the right
# include directories?
target_include_directories(spectrum PRIVATE ${fftw3_SOURCE_DIR}/api)

diy_cc_test(spectrum_test AUTO)

diy_cc_library(input_source AUTO LIBRARIES diy_coro buffer miniaudio
                                           absl::cleanup)
diy_cc_test(input_source_test AUTO)
