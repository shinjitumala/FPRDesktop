/// @file ABar.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Config.hpp>
#include <fprd/draw/Bar.hpp>

namespace fprd {

template <Orientation o, Direction d, cairo::source Frame = Color,
          cairo::source Empty = Color, cairo::source Filled = Color>
class AnimatedBar : public Bar<o, d, Frame, Empty, Filled> {
    using Base = Bar<o, d, Frame, Empty, Filled>;
    using Base::draw;

    float current;    // The currently drawn percentage.
    float increment;  // The increment per draw call.

   public:
    AnimatedBar(Base b) : Base{b}, current{0}, increment{0} {};

    void update(float target_percentage) {
        increment = (target_percentage - current) / fps;
    }

    void draw(Window& w) {
        Base::draw(w, current);
        current += increment;
    }

    [[nodiscard]] float current_percentage() const { return current; }
};
};  // namespace fprd