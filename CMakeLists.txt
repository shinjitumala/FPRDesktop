cmake_minimum_required(VERSION 3.18.3 FATAL_ERROR)

project(
  FPRDesktop
  VERSION 0.0
  DESCRIPTION "Performance monitor for FPR's personal use."
  LANGUAGES C CXX)

# Documentation
find_package(Doxygen REQUIRED dot)
set(DOXYGEN_REFERENCED_BY_RELATION YES)
set(DOXYGEN_SOURCE_BROWSER YES)
set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_EXTRACT_PRIVATE YES)
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_STRIP_CODE_COMMENTS NO)
doxygen_add_docs(doc src)

# Main library
add_library(libfprd INTERFACE)
target_compile_features(libfprd INTERFACE cxx_std_20)
target_include_directories(libfprd INTERFACE src ${CMAKE_CURRENT_BINARY_DIR}/src /usr/include/freetype2)
target_compile_options(libfprd INTERFACE -fno-rtti -fno-exceptions -Wall -g3)
string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
if(build_type MATCHES debug)
  message(STATUS "Debug build.")
  message(STATUS " Enabled sanitizers: address, undefined.")
  target_compile_options(libfprd INTERFACE -fsanitize=address,undefined)
  target_link_options(libfprd INTERFACE -fsanitize=address,undefined)
endif()
if(build_type MATCHES release)
  message(STATUS "Release build.")
  target_compile_options(libfprd INTERFACE)
  target_link_options(libfprd INTERFACE)
endif()
target_link_libraries(libfprd INTERFACE X11 cairo pthread /opt/cuda/lib64/stubs/libnvidia-ml.so LLVM)
add_dependencies(libfprd doc)

# Executable
add_executable(fprd src/main.cpp)
target_link_libraries(fprd PRIVATE libfprd)

# Configured files
configure_file(src/fprd/Config.cmake.hpp ${CMAKE_CURRENT_BINARY_DIR}/src/fprd/Config.hpp)
