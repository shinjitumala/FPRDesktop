cmake_minimum_required(VERSION 3.18.3 FATAL_ERROR)

project(
  FPRDesktop
  VERSION 0.0
  DESCRIPTION "Desktop printer for FPR's personal use."
  LANGUAGES C CXX)

# Development tools
find_package(Doxygen REQUIRED dot) # Documentation

# List of products
add_executable(fprdesktop src/main.cpp)

# Target properties
target_compile_features(fprdesktop PUBLIC cxx_std_20)
target_include_directories(
  fprdesktop PRIVATE src ${CMAKE_CURRENT_BINARY_DIR}/src /usr/include/freetype2)
# compile and link flags
target_compile_options(fprdesktop PUBLIC -fno-rtti -fno-exceptions -Wall -g3
                                         -O2)
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
  target_compile_options(fprdesktop PUBLIC -O2 -fsanitize=address,undefined -pg
                                           -flto)
  target_link_options(fprdesktop PUBLIC -O2 -fsanitize=address,undefined -pg
                      -flto)
endif()
if(${CMAKE_BUILD_TYPE} MATCHES RELEASE)
  target_compile_options(fprdesktop PUBLIC -flto)
  target_link_options(fprdesktop PUBLIC -flto)
endif()
target_link_libraries(fprdesktop PRIVATE tbb X11 cairo pthread
                                         /opt/cuda/lib64/stubs/libnvidia-ml.so)

# Sourcesset(DOXYGEN_REFERENCED_BY_RELATION YES)
set(DOXYGEN_SOURCE_BROWSER YES)
set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_EXTRACT_PRIVATE YES)
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_STRIP_CODE_COMMENTS NO)
doxygen_add_docs(doc ALL src)

# Configured files
configure_file(src/fprd/Config.cmake.hpp
               ${CMAKE_CURRENT_BINARY_DIR}/src/fprd/Config.hpp)
