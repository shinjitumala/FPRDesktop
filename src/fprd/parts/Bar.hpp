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

namespace fprd {

template <bool vertical, Source Frame = Color, Source Empty = Color,
          Source Filled = Color>
struct Bar {
    Position<float> bar;
    Area<float> s_bar;
    Position<float> bar_internal;
    Area<float> s_bar_internal;

    Frame frame;
    Empty empty;
    Filled filled;

    Bar() = default;

    void move_to(Position<float> pos, Area<float> area, Margin<float> to_bar,
                 Margin<float> to_internal_bar) {
        bar = pos.pad(to_bar);
        s_bar = area.pad(to_bar);
        bar_internal = bar.pad(to_internal_bar);
        s_bar_internal = s_bar.pad(to_internal_bar);
    }

    void set_colors(Frame frame, Empty empty, Filled filled) {
        this->frame = frame;
        this->empty = empty;
        this->filled = filled;
    }

    void update(FPRWindow &w, double percent) const {
        w.rectangle(bar, s_bar);
        w.set_source(frame);
        w.fill();
        w.rectangle(bar_internal, s_bar_internal);
        w.set_source(empty);
        w.fill();
        if constexpr (vertical) {
            auto p{bar_internal};
            p.y += s_bar_internal.h * (100 - percent) / 100;
            w.rectangle(p, s_bar_internal.scale({1, percent / 100}));
        } else {
            w.rectangle(bar_internal, s_bar_internal.scale({percent / 100, 1}));
        }
        w.set_source(filled);
        w.fill();
    }
};

template <bool vertical, Source Frame = Color, Source Empty = Color,
          Source Filled = Color>
struct BarSmooth : public Bar<vertical, Frame, Empty, Filled> {
   private:
    using Base = Bar<vertical, Frame, Empty, Filled>;

   public:
    /// The currently displayed percentage.
    float current{0};
    /// The targeted percentage.
    float target{0};

    BarSmooth() = default;

    void update_target(float percent) { target = percent; }
    void update(FPRWindow &w) {
        current = theme::slow_update(current, target);
        Base::update(w, current);
    };
};
};  // namespace fprd