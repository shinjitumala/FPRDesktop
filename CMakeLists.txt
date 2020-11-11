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
target_include_directories(fprdesktop PRIVATE src)
target_link_libraries(fprdesktop PRIVATE tbb # for parallel for loops.
)
# compile and link flags
list(APPEND fprdesktop_compile_options -fno-rtti -fno-exceptions -Wall -g3)
list(APPEND fprdesktop_link_options)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # Use LTO if clang is available.
  list(APPEND fprdesktop_compile_options -flto=thin)
  list(APPEND fprdesktop_link_options -flto=thin)
endif()
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug") # Sanitize everything in debug mode
  list(APPEND fprdesktop_compile_options -fsanitize=address,undefined)
  list(APPEND fprdesktop_link_options -fsanitize=address,undefined)
endif()
target_compile_options(fprdesktop PUBLIC ${fprdesktop_compile_options})
target_link_options(fprdesktop PUBLIC ${fprdesktop_link_options})

# Sources
doxygen_add_docs(doc src include)
target_sources(fprdesktop PRIVATE src/main.cpp)
