cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)

# Adding FFTW as a dependency causes absl to fail to link without this.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Flags shared between C and C++ targets
add_compile_options(-O0 -g3)
# Flags only for C++ targets
set(cxx_flags -ftemplate-backtrace-limit=0 -fconcepts-diagnostics-depth=10)
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${cxx_flags}>")

project(host_gui C CXX)

include(FetchContent)

set(ABSL_PROPAGATE_CXX_STD ON)
FetchContent_Declare(
  absl
  GIT_REPOSITORY "https://github.com/abseil/abseil-cpp.git"
  GIT_TAG "20220623.1")

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1)

FetchContent_Declare(fftw3 URL "https://www.fftw.org/fftw-3.3.10.tar.gz")

FetchContent_MakeAvailable(absl googletest fftw3)
include(GoogleTest)
enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_subdirectory(diy)
add_subdirectory(audio)
add_subdirectory(gui)

add_executable(main main.cc)
target_link_libraries(main PRIVATE Qt6::Widgets main_window)

add_custom_target(
  format_cc
  COMMAND find ${PROJECT_SOURCE_DIR} -type f | grep -v ${PROJECT_BINARY_DIR} |
          grep -E "(cc|h)$" | xargs -L1 clang-format -i
  VERBATIM)
add_custom_target(
  format_cmake
  COMMAND find ${PROJECT_SOURCE_DIR} -name CMakeLists.txt | grep -v
          ${PROJECT_BINARY_DIR} | xargs -L1 cmake-format -i
  VERBATIM)
add_custom_target(format DEPENDS format_cc format_cmake)

add_custom_target(
  itest
  COMMAND find ${PROJECT_SOURCE_DIR} -type f | grep -v ${PROJECT_BINARY_DIR} |
          entr -r sh -c 'make -j && ctest --output-on-failure')
