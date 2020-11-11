/// @file ostream.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary
/// You are not allowed to use this file
/// without the author's permission.

#pragma once

#include <concepts>
#include <dbg/Logger.hpp>
#include <experimental/source_location>
#include <iomanip>
#include <ostream>
#include <type_traits>
#include <vector>

namespace fprd {
using namespace std;
using source_location = ::std::experimental::source_location;

/// For convenience.
constexpr string_view nl{"\n"};

/// Prinable types have a print function for types defined in SWORD.
/// @tparam T
template <class T> concept Printable = requires(const T &t, ostream &os) {
    { t.print(os) }
    ->same_as<ostream &>;
};

/// An operator overload for printable types defined by SWORD.
/// @tparam P
/// @param os
/// @param p
/// @return ostream&
template <Printable P> ostream &operator<<(ostream &os, const P &p) {
    return p.print(os);
};

/// Constants used for printing file location.
constexpr auto filename_len{16};
constexpr auto num_len{3};
constexpr auto location_len{filename_len + 2 * num_len + 2};

/// An abstract function for printing file locations.
/// @param os
/// @param line
/// @param column
/// @param filename
/// @return ostream&
ostream &print_location(ostream &os, const uint line, const uint column,
                        const string_view filename) {
    if (filename.size() > filename_len) {
        const auto short_filename{filename.substr(
            filename.size() - filename_len + 3, filename_len - 3)};
        os << setw(filename_len - short_filename.size()) << setfill(' ')
           << right << "..." << short_filename;
    } else {
        os << setw(filename_len) << setfill(' ') << right << filename;
    }
    os << ":";
    os << setw(num_len) << setfill('0') << line;
    os << ":";
    os << setw(num_len) << setfill('0') << column;
    return os;
}

/// Operator overload for printing source code location info.
/// @param os
/// @param loc
/// @return constexpr ostream&
ostream &operator<<(ostream &os, const source_location loc) {
    return print_location(os, loc.line(), loc.column(), loc.file_name());
}

/// Operator overload for vector.
template <class T, class A>
ostream &operator<<(ostream &os, const vector<T, A> &vec) {
    os << "{" << nl;
    {
        dbg::IndentGuard ig{};
        for (auto &e : vec) {
            os << e << "," << nl;
        }
    }
    os << "}";
    return os;
}
}; // namespace fprd