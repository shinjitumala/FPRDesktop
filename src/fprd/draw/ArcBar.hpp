/// @file ArcBar.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Theme.hpp>
#include <fprd/Types.hpp>
#include <fprd/Window.hpp>
#include <fprd/wrapper/Cairo.hpp>

namespace fprd {

/// The direction the bar should be filled.
enum class ArcBarDirection {
    clock_wise,
    counter_clock_wise,
};

/// A circular bar.
struct ArcBarBase {
    /// The center of the circle.
    Position<float> center;
    /// The radius of the circle.
    float radious;
    /// The start angle in radians.
    float start;
    /// The end angle in radians.
    float end;

    float border_width;
    float bar_width;
};

/// The way The bar is drawn changes depending on the direction.
/// @tparam d
/// @tparam Border
/// @tparam Empty
/// @tparam Filled
template <ArcBarDirection d, cairo::source Border = Color,
          cairo::source Empty = Color, cairo::source Filled = Color>
struct ArcBar : public ArcBarBase {
    Border border;
    Empty empty;
    Filled filled;

    /// Draw the ArcBar with its current data.
    /// @param w
    void draw(Window &w, float filled_percent) {
        w.set_line_width(bar_width - border_width * 2);
        w.arc(center, radious - bar_width / 2, start, start);
        w.fill();
        w.set_source(empty);
        arc<true>(w, center, radious - bar_width / 2, start, end);
        w.stroke();
        w.set_source(filled);
        arc<true>(w, center, radious - bar_width / 2, start,
                  start + (end - start) * filled_percent / 100);
        w.stroke();
        w.set_source(border);
        w.set_line_width(border_width);
        arc<true>(w, center, radious - border_width / 2, start, end);
        arc<false>(w, center, radious - bar_width + border_width / 2, end,
                   start);
        w.close_path();
        w.stroke();
    }

   private:
    /// Internal utility function.
    /// @tparam positive As in the fill direction. Dependant on Direction d.
    /// @param w
    /// @param center
    /// @param radious
    /// @param start
    /// @param end
    template <bool positive>
    void arc(Window &w, Position<float> center, float radious, float start,
             float end) {
        if constexpr ((d == ArcBarDirection::clock_wise && positive) ||
                      (d == ArcBarDirection::counter_clock_wise && !positive)) {
            w.arc(center, radious, start, end);
        } else {
            w.rarc(center, radious, start, end);
        }
    }
};
}  // namespace fprd