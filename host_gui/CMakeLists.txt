cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 20)

# Adding FFTW as a dependency causes absl to fail to link without this.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Flags shared between C and C++ targets
add_compile_options(-O2 -g3 -ggdb -march=skylake)
# Flags only for C++ targets
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-ftemplate-backtrace-limit=0>)
add_compile_options(
  $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fconcepts-diagnostics-depth=10>)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(STATUS "Enabling ccache")
  list(APPEND launchers "${CCACHE_PROGRAM}")
else()
  message(STATUS "ccache not found")
endif()

option(MIC_ENABLE_DISTCC "Enable distcc." OFF)
find_program(DISTCC_PROGRAM distcc)
if(DISTCC_PROGRAM)
  if(MIC_ENABLE_DISTCC)
    message(STATUS "Enabling distcc")
    list(APPEND launchers "${DISTCC_PROGRAM}")
  endif()
else()
  message(STATUS "distcc not found")
endif()

set(CMAKE_C_COMPILER_LAUNCHER "${launchers}")
set(CMAKE_CXX_COMPILER_LAUNCHER "${launchers}")

project(host_gui C CXX)

include(FetchContent)

set(ABSL_PROPAGATE_CXX_STD ON)
FetchContent_Declare(
  absl
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG 522606b7fae37836c138e83f6eec0eabb9947dc0)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.7.1)

set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1)

FetchContent_Declare(fftw3 URL https://www.fftw.org/fftw-3.3.10.tar.gz)

FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.4)

# Using MakeAvailable causes many expensive Eigen tests to be enabled. Eigen is
# header-only, so we don't need to fully build it.
FetchContent_Populate(eigen)
add_library(eigen INTERFACE)
target_include_directories(eigen INTERFACE ${eigen_SOURCE_DIR})

FetchContent_Declare(
  miniaudio
  GIT_REPOSITORY https://github.com/mackron/miniaudio.git
  GIT_TAG c153a947919808419b0bf3f56b6f2ee606d6c5f4)

FetchContent_MakeAvailable(absl benchmark fftw3 googletest miniaudio)

add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE ${miniaudio_SOURCE_DIR})

FetchContent_Declare(diy_coro URL /home/dhrosa/diy/coro)
FetchContent_MakeAvailable(diy_coro)

include(GoogleTest)
enable_testing()
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

option(MIC_ENABLE_OPENMP "Enable OpenMP for certain operations." OFF)

if(MIC_ENABLE_OPENMP)
  message(STATUS "OpenMP enabled.")
  find_package(OpenMP REQUIRED)
else()
  message(STATUS "OpenMP disabled.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Diy)

include_directories(.)

add_subdirectory(diy)
add_subdirectory(audio)
add_subdirectory(image)
add_subdirectory(gui)

diy_cc_binary(
  main AUTO LIBRARIES Qt6::Widgets main_window absl::failure_signal_handler
                      absl::log_initialize absl::log_globals)

add_custom_target(
  format_cc
  COMMAND find ${PROJECT_SOURCE_DIR} -type f | grep -v ${PROJECT_BINARY_DIR} |
          grep -E "[.](h|cc)$" | xargs -L1 clang-format -i
  VERBATIM)
add_custom_target(
  format_cmake
  COMMAND find ${PROJECT_SOURCE_DIR} -name CMakeLists.txt | grep -v
          ${PROJECT_BINARY_DIR} | xargs -L1 cmake-format -i
  VERBATIM)
add_custom_target(format DEPENDS format_cc format_cmake)
