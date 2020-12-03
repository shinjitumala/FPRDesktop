/// @file ArcBar.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Theme.hpp>
#include <fprd/Utils.hpp>
#include <fprd/Window.hpp>

namespace fprd {
struct ArcBarBase {
    Position<float> center;
    float radious;
    float start;
    float end;

    float border_width;
    float bar_width;

    Color border;
    Color bg;
    Color fg;

    float current{0};  // %
    float target{0};   // %
};

template <bool clock_wise>
struct ArcBar : public ArcBarBase {
    void draw(FPRWindow &w) {
        current = theme::slow_update(current, target);

        w.set_line_width(bar_width - border_width * 2);
        w.arc(center, radious - bar_width / 2, start, start);
        w.reset();
        w.set_source(bg);
        arc<true>(w, center, radious - bar_width / 2, start, end);
        w.stroke();
        w.set_source(fg);
        arc<true>(w, center, radious - bar_width / 2, start,
                  start + (end - start) * current / 100);
        w.stroke();
        w.set_source(border);
        w.set_line_width(border_width);
        arc<true>(w, center, radious - border_width / 2, start, end);
        arc<false>(w, center, radious - bar_width + border_width / 2, end,
                   start);
        w.close_path();
        w.stroke();
    }

    template <bool b>
    ArcBar &operator=(ArcBar<b> rhs) {
        *static_cast<ArcBarBase *>(this) = rhs;
        return *this;
    }

   private:
    template <bool forward>
    void arc(FPRWindow &w, Position<float> center, float radious, float start,
             float end) {
        if constexpr ((clock_wise && forward) || (!clock_wise && !forward)) {
            w.arc(center, radious, start, end);
        } else {
            w.rarc(center, radious, start, end);
        }
    }
};
}  // namespace fprd