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

/// Animated version of ArcBar.
/// Update should be called every second.
/// @tparam d
/// @tparam Border
/// @tparam Empty
/// @tparam Filled
template <ArcBarDirection d, cairo::source Border = Color,
          cairo::source Empty = Color, cairo::source Filled = Color>
class AnimatedArcBar : ArcBar<d, Border, Empty, Filled> {
    using Base = ArcBar<d, Border, Empty, Filled>;
    using Base::draw;

    float current{0};    // The currently drawn percentage.
    float increment{0};  // The increment per draw call.

   public:
    /// Default constructor.
    AnimatedArcBar() = default;
    /// Copying is not allowed.
    AnimatedArcBar(const AnimatedArcBar&) = delete;
    /// Moving is allowed, however.
    AnimatedArcBar(AnimatedArcBar&&) noexcept = default;
    /// Needed for some syntax sugar.
    AnimatedArcBar& operator=(const AnimatedArcBar&) = default;
    /// Initialize from an ArcBar.
    /// @param arc_bar
    AnimatedArcBar(Base arc_bar) : Base{arc_bar} {}

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