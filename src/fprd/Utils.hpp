/// @file Utils.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

namespace fprd {

/// For readability.
struct Margin {
    double x;
    double y;

    Margin operator+(Margin m) const { return {x + m.x, y + m.y}; };
    Margin operator*(float s) const { return {x * s, y * s}; };
};

/// For readability.
struct Size {
    double w;
    double h;

    Size operator-(Margin m) const { return {w - m.x * 2, h - m.y * 2}; };

    [[nodiscard]] Size scale(pair<double, double> scale) const {
        return {w * scale.first, h * scale.second};
    };

    auto operator<=>(const Size &) const = default;
};

/// For readability.
struct Position {
    double x;
    double y;

    Position operator-(Margin m) const { return {x + m.x, y + m.y}; }
    Position operator+(Position p) const { return {x + p.x, y + p.y}; }
};
}; // namespace fprd