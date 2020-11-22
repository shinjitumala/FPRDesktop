/// @file Color.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <dbg/Log.hpp>
#include <string>
#include <tuple>

namespace fprd {
using namespace std;

/// Convenience class for expressing color.
struct Color {
    double r;
    double g;
    double b;
    double a;

    Color() = default;

    /// Initialize from raw doubles.
    /// @param r
    /// @param g
    /// @param b
    /// @param a
    Color(double r, double g, double b, double a = 1.0F)
        : r{r}, g{g}, b{b}, a{a} {}

    /// Initialize from hex string (the string you use in HTML).
    /// @param hex
    /// @param a
    Color(string hex, double a = 1.0F)
        : Color{[hex]() {
                    dbg(if (hex.size() != 6) {
                        fatal_error("Not a HEX color code?");
                    });

                    return make_tuple(
                        (double)stoi(hex.substr(0, 2), nullptr, 16) / 255,
                        (double)stoi(hex.substr(2, 2), nullptr, 16) / 255,
                        (double)stoi(hex.substr(4, 2), nullptr, 16) / 255);
                }(),
                a} {}

    /// Print in a json-like format.
    /// @param os
    /// @return ostream&
    ostream &print(ostream &os) const {
        os << "{ ";
        os << r << ", " << g << ", " << b << ", " << a;
        os << " }";
        return os;
    }

   private:
    Color(tuple<double, double, double> rgb, double a)
        : r{get<0>(rgb)}, g{get<1>(rgb)}, b{get<2>(rgb)}, a{a} {}
};
};  // namespace fprd