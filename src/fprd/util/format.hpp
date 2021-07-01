/// @file format.hpp
/// @author FPR (funny.pig.run__AT_MARK_gmail.com)
/// @version 0.0
/// @date 2021-07-01
///
/// @copyright Copyright (c) 2021
///

#pragma once

#include <array>
#include <string_view>

namespace fprd {
using namespace std;
/// `print_to` but with offset.
/// Example:
///   Passing arguments; dst = "foofoo", offset = 3, fmt = "bar"
///   will result in dst = "foobar".
/// @tparam S
/// @tparam Args
/// @param dst
/// @param offset
/// @param fmt
/// @param args
/// @return auto
template <size_t S, class... Args>
auto print_to(array<char, S> &dst, size_t offset, string_view fmt, Args... args) -> auto {
    return snprintf(dst.data() + offset, dst.size() - offset, fmt.data(), args...);
}
/// C++ version of `snprintf` that uses array<char, S>
/// @tparam S
/// @tparam Args
/// @param dst
/// @param fmt
/// @param args
/// @return auto
template <size_t S, class... Args> auto print_to(array<char, S> &dst, string_view fmt, Args... args) -> auto {
    return print_to(dst, 0, fmt, args...);
}

/// Space terminated version of `print_to`.
/// The other version is null terminated.
/// @tparam S
/// @tparam Args
/// @param dst
/// @param offset
/// @param fmt
/// @param args
/// @return auto
template <size_t S, class... Args>
auto print_to_st(array<char, S> &dst, size_t offset, string_view fmt, Args... args) -> auto {
    const auto np{print_to(dst, offset, fmt, args...)};
    dst[np + offset] = ' ';
    return np;
}

template <size_t S, class... Args> auto print_to_st(array<char, S> &dst, string_view fmt, Args... args) -> auto {
    return print_to_st(dst, 0, fmt, args...);
}
}; // namespace fprd