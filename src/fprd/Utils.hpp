/// @file Utils.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <utility>

namespace fprd {
using namespace std;

template <class I> concept Number = is_integral_v<I> || is_floating_point_v<I>;

/// For readability.
template <Number I = double> struct Margin {
    I x;
    I y;

    Margin operator+(Margin m) const { return {x + m.x, y + m.y}; };
    Margin operator*(float s) const { return {x * s, y * s}; };
};

/// For readability.
template <Number I = double> struct Size {
    I w;
    I h;

    Size operator-(Margin<I> m) const { return {w - m.x * 2, h - m.y * 2}; };

    [[nodiscard]] Size scale(pair<double, double> scale) const {
        return {w * scale.first, h * scale.second};
    };

    auto operator<=>(const Size &) const = default;
};

/// For readability.
template <Number I = double> struct Position {
    I x;
    I y;

    Position operator-(Margin<I> m) const { return {x + m.x, y + m.y}; }
    Position operator+(Position p) const { return {x + p.x, y + p.y}; }
};
}; // namespace fprd