cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)

# Adding FFTW as a dependency causes absl to fail to link without this.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Flags shared between C and C++ targets
add_compile_options(-O0 -g3)
# Flags only for C++ targets
set(cxx_flags -ftemplate-backtrace-limit=0 -fconcepts-diagnostics-depth=10)
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${cxx_flags}>")

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  message(STATUS "Using ccache found at ${CCACHE_PROGRAM}")
else()
  message(STATUS "ccache not found.")
endif()

project(host_gui C CXX)

include(FetchContent)

set(ABSL_PROPAGATE_CXX_STD ON)
FetchContent_Declare(
  absl
  GIT_REPOSITORY "https://github.com/abseil/abseil-cpp.git"
  GIT_TAG 296abbd35a600785d6297738749336061ecb2221)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1)

FetchContent_Declare(fftw3 URL "https://www.fftw.org/fftw-3.3.10.tar.gz")

FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.4)

# Using MakeAvailable causes many expensive Eigen tests to be enabled. Eigen is
# header-only, so we don't need to fully build it.
FetchContent_Populate(eigen)
set(EIGEN_SOURCE_DIR ${eigen_SOURCE_DIR})

FetchContent_MakeAvailable(absl fftw3 googletest)
include(GoogleTest)
enable_testing()
find_package(Qt6 REQUIRED COMPONENTS Widgets)

include_directories(.)

add_subdirectory(diy)
add_subdirectory(audio)
add_subdirectory(image)
add_subdirectory(gui)

add_executable(main main.cc)
target_link_libraries(main PUBLIC Qt6::Widgets main_window absl::log_initialize
                                  absl::log_globals)

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
