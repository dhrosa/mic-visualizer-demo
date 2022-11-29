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

add_library(audio_source source.cc source.h)
target_link_libraries(audio_source cardinal generator absl::time)

diy_add_test(audio_source_test source_test.cc)
target_link_libraries(audio_source_test audio_source)

add_library(audio_spectrum spectrum.h spectrum.cc)
# For some reason the fftw3 library itself doesn't automatically add the right
# include directories?
target_include_directories(audio_spectrum PRIVATE ${fftw3_SOURCE_DIR}/api)
target_link_libraries(audio_spectrum fftw3 absl::cleanup)

diy_add_test(audio_spectrum_test spectrum_test.cc)
target_link_libraries(audio_spectrum_test audio_spectrum)
