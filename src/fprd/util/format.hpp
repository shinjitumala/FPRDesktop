/// @file format.hpp
/// @author FPR (funny.pig.run__AT_MARK_gmail.com)
/// @version 0.0
/// @date 2021-07-01
///
/// @copyright Copyright (c) 2021
///

#pragma once

#include <array>
#include <cstdio>
#include <span>
#include <string_view>

namespace fprd {
using namespace std;
/// C++ version of `snprintf` that uses array<char, S>
/// @tparam S
/// @tparam Args
/// @param dst
/// @param fmt
/// @param args
/// @return auto
template <size_t S, class... Args> auto snprintf(array<char, S> &dst, string_view fmt, Args... args) -> auto {
    return ::snprintf(dst.data(), dst.size(), fmt.data(), args...);
}

/// Print to a range of character using a space character terminator.
/// @tparam Args 
/// @param dst 
/// @param fmt 
/// @param args 
/// @return auto 
template <class... Args> auto snprintf_st(span<char> dst, string_view fmt, Args... args) -> auto {
    const auto np{::snprintf(dst.data(), dst.size(), fmt.data(), args...)};
    dst[np] = ' ';
    return np;
}
}; // namespace fprd