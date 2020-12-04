cmake_minimum_required(VERSION 3.18.3 FATAL_ERROR)

project(
  FPRDesktop
  VERSION 0.0
  DESCRIPTION "Desktop printer for FPR's personal use."
  LANGUAGES C CXX)

# Development tools
find_package(Doxygen REQUIRED dot) # Documentation

# List of products
add_executable(fprdesktop)

# Target properties
target_compile_features(fprdesktop PUBLIC cxx_std_20)
target_include_directories(
  fprdesktop PRIVATE src ${CMAKE_CURRENT_BINARY_DIR}/src /usr/include/freetype2)
# compile and link flags
list(APPEND fprdesktop_compile_options -fno-rtti -fno-exceptions -Wall -g3)
list(APPEND fprdesktop_link_options)
list(APPEND fprdesktop_link_libraries tbb X11 cairo pthread
     /opt/cuda/lib64/stubs/libnvidia-ml.so)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # Use LTO if clang is available.
  list(APPEND fprdesktop_compile_options -flto=thin)
  list(APPEND fprdesktop_link_options -flto=thin)
endif()
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug") # Sanitize everything in debug mode
  list(APPEND fprdesktop_compile_options -fsanitize=address,undefined)
  list(APPEND fprdesktop_link_options -fsanitize=address,undefined)
  list(APPEND fprdesktop_link_libraries profiler)
endif()
target_compile_options(fprdesktop PUBLIC ${fprdesktop_compile_options})
target_link_options(fprdesktop PUBLIC ${fprdesktop_link_options})
target_link_libraries(fprdesktop PRIVATE ${fprdesktop_link_libraries})

# Sources
doxygen_add_docs(doc src include)
target_sources(fprdesktop PRIVATE src/main.cpp)
target_precompile_headers(fprdesktop PRIVATE src/dbg/Log.hpp
                          src/fprd/Window.hpp)

# Configured files
configure_file(src/fprd/Config.cmake.hpp
               ${CMAKE_CURRENT_BINARY_DIR}/src/fprd/Config.hpp)
