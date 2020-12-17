/// @file AArcBar.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Config.hpp>
#include <fprd/draw/ArcBar.hpp>

namespace fprd {

template <ArcBarDirection d, cairo::source Border = Color,
          cairo::source Empty = Color, cairo::source Filled = Color>
class AnimatedArcBar : ArcBar<d, Border, Empty, Filled> {
    using Base = ArcBar<d, Border, Empty, Filled>;
    using Base::draw;

    float current;    // The currently drawn percentage.
    float increment;  // The increment per draw call.

   public:
    AnimatedArcBar(Base arc_bar) : Base{arc_bar}, current{0}, increment{0} {}

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