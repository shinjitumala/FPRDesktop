#!/bin/sh
cd "$(dirname "$0")"
find src -type f -exec sed -i 's/#include \"\([^"]*\)\"/#include <\1>/g' -- {} \; -exec clang-format -i -- {} \;
cmake-format -i CMakeLists.txt