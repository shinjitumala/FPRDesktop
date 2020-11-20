/// @file Bar.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Window.hpp>

namespace fprd {

template <Source Frame = Color, Source Empty = Color, Source Filled = Color>
struct Bar {
    Position bar;
    Size s_bar;
    Position bar_internal;
    Size s_bar_internal;

    Frame frame;
    Empty empty;
    Filled filled;

    Bar(Position pos, Size area, Margin to_bar, Margin to_internal_bar,
        Frame &&frame = {}, Empty &&empty = {}, Filled &&filled = {})
        : bar{pos - to_bar}, s_bar{area - to_bar},
          bar_internal{bar - to_internal_bar}, s_bar_internal{s_bar -
                                                              to_internal_bar},
          frame{move(frame)}, empty{move(empty)}, filled{move(filled)} {}

    void update(FPRWindow &w, double percent) const {
        w.rectangle(bar, s_bar);
        w.set_source(frame);
        w.fill();
        w.rectangle(bar_internal, s_bar_internal);
        w.set_source(empty);
        w.fill();
        w.rectangle(bar_internal, s_bar_internal.scale({percent / 100, 1}));
        w.set_source(filled);
        w.fill();
    }
};

template <Source Frame = Color, Source Empty = Color, Source Filled = Color>
struct BarSmooth : protected Bar<Frame, Empty, Filled> {
  private:
    using Base = Bar<Frame, Empty, Filled>;

  public:
    /// The currently displayed percentage.
    float current{};
    /// The targeted percentage.
    float target{};
    /// Smoothness (Higher is smoother)
    short smoothness;

    BarSmooth(Base &&bar, short smoothness)
        : Base{move(bar)}, smoothness{smoothness} {}

    void update_target(float percent) { target = percent; }
    void update(FPRWindow &w) {
        current = current + (float)(target - current) / smoothness;
        Base::update(w, current);
    };
};
}; // namespace fprd