/// @file to_string.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <iomanip>
#include <ios>
#include <sstream>
#include <string>

namespace fprd {
using namespace std;

/// to_string for floating point values that sets the precision.
/// @tparam Float
/// @param f
/// @return auto
template <u_char precision, class Float> auto ftos(Float f) requires is_floating_point_v<Float> {
    ostringstream oss;
    oss << setprecision(precision) << fixed << f;
    return oss.str();
}

/// Obtains string padded with spaces to the specified width.
/// Spaces are added to the left of the string.
/// @tparam w
/// @param s
/// @return auto
template <size_t w> auto width(string &&s) {
    ostringstream oss;
    oss << setfill(' ') << setw(w) << s;
    return oss.str();
}

template <size_t maxw> string truncs(string_view s) {
    if (s.size() < maxw) {
        return string{s};
    }
    return string{s.substr(0, maxw - 3)} + "...";
};
}; // namespace fprd