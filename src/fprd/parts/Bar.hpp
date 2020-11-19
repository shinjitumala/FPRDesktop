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
class Bar {
    Position bar;
    Size s_bar;
    Position bar_internal;
    Size s_bar_internal;

  public:
    Frame frame;
    Empty empty;
    Filled filled;

    Bar(Position pos, Size area, Margin to_bar, Margin to_internal_bar)
        : bar{pos - to_bar}, s_bar{area - to_bar},
          bar_internal{bar - to_internal_bar}, s_bar_internal{
                                                   s_bar - to_internal_bar} {}

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
}; // namespace fprd