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

diy_cc_library(source AUTO LIBRARIES cardinal generator buffer absl::time)

diy_cc_test(source_test AUTO)

diy_cc_library(spectrum AUTO LIBRARIES fftw3 generator buffer)
# For some reason the fftw3 library itself doesn't automatically add the right
# include directories?
target_include_directories(spectrum PRIVATE ${fftw3_SOURCE_DIR}/api)

diy_cc_test(spectrum_test AUTO)

diy_cc_library(
  qaudio_source_coro AUTO LIBRARIES Qt6::Multimedia async_generator buffer
                                    absl::synchronization)
diy_cc_test(qaudio_source_coro_test AUTO)
