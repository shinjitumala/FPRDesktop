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

/// Animated version of Bar.
/// @tparam o
/// @tparam d
/// @tparam Frame
/// @tparam Empty
/// @tparam Filled
template <Orientation o, Direction d, cairo::source Frame = Color,
          cairo::source Empty = Color, cairo::source Filled = Color>
class AnimatedBar : public Bar<o, d, Frame, Empty, Filled> {
    using Base = Bar<o, d, Frame, Empty, Filled>;
    using Base::draw;

    float current{0};    // The currently drawn percentage.
    float increment{0};  // The increment per draw call.

   public:
    /// Default constructor.
    AnimatedBar() = default;
    /// Copying is not allowed.
    AnimatedBar(const AnimatedBar&) = delete;
    /// Moving is allowed, however.
    AnimatedBar(AnimatedBar&&) noexcept = default;
    /// Initialzation from a Bar.
    /// @param b
    AnimatedBar(Base b) : Base{b} {};

    /// Update the target percentage.
    /// Currently hard-coded such that this function must be called every second
    /// or it breaks.
    /// TODO: Make the update interval adjustable via template parameters.
    /// @param target_percentage
    void update(float target_percentage) {
        increment = (target_percentage - current) / fps;
    }

    /// Call this every frame.
    /// @param w
    void draw(Window& w) {
        Base::draw(w, current);
        current += increment;
    }

    /// In case you need to peek the current value of the bar.
    /// @return float
    [[nodiscard]] float current_percentage() const { return current; }
};
};  // namespace fprd