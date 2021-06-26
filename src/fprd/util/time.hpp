/// @file time.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <chrono>

namespace fprd {
using namespace ::std;
using namespace ::std::chrono;

/// Get the current time.
/// @return auto
auto now() {
    using namespace ::std::chrono;
    return time_point<high_resolution_clock>::clock::now();
}

/// Gets time difference in milliseconds.
/// @param tp
/// @return auto
auto diff(const chrono::time_point<chrono::high_resolution_clock> &tp) {
    using namespace std::chrono;
    return duration_cast<milliseconds>(now() - tp).count();
}
} // namespace fprd