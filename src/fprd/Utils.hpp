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

template <class I>
concept Number = is_integral_v<I> || is_floating_point_v<I>;

/// For readability.
template <Number I = double>
struct Margin {
    I x;
    I y;

    Margin operator+(Margin m) const { return {x + m.x, y + m.y}; };
    Margin operator*(float s) const { return {x * s, y * s}; };

    template <class T>
    operator Margin<T>() requires is_convertible_v<I, T> {
        return {x, y};
    }
};

/// For readability.
template <Number I = double>
struct Size {
    I w;
    I h;

    Size pad(Margin<I> m) const { return {w - m.x * 2, h - m.y * 2}; };

    [[nodiscard]] Size scale(pair<double, double> scale) const {
        return {w * scale.first, h * scale.second};
    };

    auto operator<=>(const Size &) const = default;

    template <class T>
    operator Size<T>() requires is_convertible_v<I, T> {
        return {w, h};
    }
};

/// For readability.
template <Number I = double>
struct Position {
    I x;
    I y;

    Position pad(Margin<I> m) const { return {x + m.x, y + m.y}; }
    Position operator+(Position p) const { return {x + p.x, y + p.y}; }
    /// Bottom left
    Position bl(Size<I> s) const { return {x, y - s.h}; };
    /// Top right
    Position tr(Size<I> s) const { return {x - s.w, y}; };
    /// Bottom right
    Position br(Size<I> s) const { return {x - s.w, y - s.h}; };
    /// Horizontal center
    Position hc(Size<I> s) const { return {x - s.w / 2, y}; };

    template <class T>
    operator Position<T>() requires is_convertible_v<I, T> {
        return {x, y};
    }
};
};  // namespace fprd