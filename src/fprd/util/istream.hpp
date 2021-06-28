/// @file istream.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <istream>
#include <numeric>

namespace fprd {
using namespace ::std;

auto skip_to(istream &is, char c) -> void { is.ignore(numeric_limits<streamsize>::max(), c); }

/// Ignore the specified number of lines.
/// @param is
/// @param lines
/// @return auto
auto skip_lines(istream &is, int lines) {
    for (auto i{0}; i < lines; i++) {
        skip_to(is, '\n');
    }
}

template <char delim> auto skip_fields(istream &is, int fields) {
    for (auto i{0}; i < fields; i++) {
        skip_to(is, delim);
    }
}

/// @param is
/// @return auto
auto getline(istream &is) {
    string s;
    getline(is, s);
    return s;
}

template <class T> auto get(istream &is) -> auto {
    T t;
    is >> t;
    return t;
}

}; // namespace fprd