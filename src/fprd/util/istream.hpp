/// @file istream.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <istream>

namespace fprd {
using namespace ::std;

/// Ignore the specified number of lines.
/// @param is
/// @param lines
/// @return auto
auto skip_lines(istream& is, unsigned char lines) {
    for (unsigned char i{0}; i < lines; i++) {
        is.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

/// @param is
/// @return auto
auto getline(istream& is) {
    string s;
    getline(is, s);
    return s;
}

/// @param is
/// @return auto
auto getstring(istream& is) {
    string s;
    is >> s;
    return s;
}

/// @param is
/// @return auto
auto getfloat(istream& is) {
    float f;
    is >> f;
    return f;
}

/// @param is
/// @return auto
auto getulong(istream& is) {
    ulong l;
    is >> l;
    return l;
}

/// @param is
/// @return auto
auto getuint(istream& is) {
    uint i;
    is >> i;
    return i;
}

/// @param is
/// @return auto
auto getint(istream& is) {
    int i;
    is >> i;
    return i;
}

/// @param is
/// @return auto
auto getchar(istream& is) {
    char c;
    is >> c;
    return c;
}

/// @param is
/// @param delim
/// @return auto
auto skip_to(istream& is, char delim) {
    is.ignore(numeric_limits<streamsize>::max(), delim);
}
};  // namespace fprd