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
using namespace ::std::chrono;

/// Location for resources
static inline const path resources{"@CMAKE_CURRENT_SOURCE_DIR@/res"};
static inline const auto data_update_interval{1s}; // Update data every second
static inline const auto fps{60};                  // Frames per second
static inline const auto draw_interval{duration_cast<microseconds>(1s) / fps};
} // namespace fprd