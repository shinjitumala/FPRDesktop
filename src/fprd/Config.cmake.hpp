/// @file Config.cmake.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <filesystem>

namespace fprd {
using namespace ::std::filesystem;
static inline const path resources{"@CMAKE_CURRENT_SOURCE_DIR@/res"};
}  // namespace fprd