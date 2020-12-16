/// @file Bar.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Theme.hpp>
#include <fprd/Window.hpp>
#include <fprd/values/animated.hpp>

namespace fprd {

enum class Orientation {
    horizontal,
    vertical,
};

enum class Direction {
    positive,
    negative,
};

template <Orientation o, Direction d, cairo::Source Frame = Color,
          cairo::Source Empty = Color, cairo::Source Filled = Color>
struct Bar {
    Position<float> pos;
    Area<float> area;
    float border_width;

    Frame frame;
    Empty empty;
    Filled filled;

    void draw(Window &w, float percent) const {
        /// Background
        w.rectangle(pos, area);
        w.set_source(empty);
        w.fill();

        /// Percentage filled
        const auto fill_area{[&] {
            if constexpr (o == Orientation::vertical) {
                return area.scale({1, percent / 100});
            }
            if constexpr (o == Orientation::horizontal) {
                return area.scale({percent / 100, 1});
            }
        }()};
        const auto fill_pos{[&] {
            if constexpr (d == Direction::positive) {
                return pos;
            }
            if constexpr (d == Direction::negative) {
                return fill_area.bottom_right(pos.stack(area));
            }
        }()};
        w.rectangle(fill_pos, fill_area);
        w.set_source(filled);
        w.fill();

        /// Border
        w.rectangle(pos, area);
        w.set_source(frame);
        w.set_line_width(border_width);
        w.stroke();
    }
};
};  // namespace fprd