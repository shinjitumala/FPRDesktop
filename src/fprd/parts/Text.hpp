/// @file MutableText.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Window.hpp>

namespace fprd {
template <Source Background = Color, Source Foreground = Color> struct Text {
    Position pos;
    Size area;
    double size;
    Position text_pos;
    const Font &f;

    Background bg;
    Foreground fg;

    Text(Position pos, Size area, Margin m, const Font &f,
         Background &&bg = {"000000"}, Foreground &&fg = {"ffffff"})
        : pos{pos}, area{area}, size{(area - m).h}, text_pos{[&]() {
              auto p{pos - m};
              p.y += size;
              return p;
          }()},
          f{f}, bg{move(bg)}, fg{move(fg)} {}

    void update(FPRWindow &w, string &&s) const {
        w.set_source(bg);
        w.rectangle(pos, area);
        w.fill();
        w.set_source(fg);
        w.move_to(text_pos);
        w.set_font_size(size);
        w.set_font(f);
        w.print_text(move(s));
    }
};
} // namespace fprd