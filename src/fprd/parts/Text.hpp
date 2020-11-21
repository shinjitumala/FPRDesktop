/// @file MutableText.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Window.hpp>
#include <optional>

namespace fprd {
enum class TextAlign : u_char {
    center,
    left,
    right,
};

template <bool clear = true, TextAlign vertical = TextAlign::left,
          Source Background = Color, Source Foreground = Color>
struct Text {
    Position<float> pos;
    Size<float> area;
    Margin<float> m;
    double size;
    Position<float> text_pos;
    const Font &f;

    Background bg;
    Foreground fg;

    Text(Position<float> pos, Size<float> area, Margin<float> m, const Font &f,
         Foreground &&fg = {"ffffff"}, Background &&bg = {"000000"})
        : pos{pos}, area{area}, m{m}, size{(area - m).h}, text_pos{[&]() {
              auto p{pos - m};
              p.y += size;
              return p;
          }()},
          f{f}, bg{move(bg)}, fg{move(fg)} {}

    void update(FPRWindow &w, string &&s) {
        if constexpr (clear) {
            w.set_source(bg);
            w.rectangle(pos, area);
            w.fill();
        }
        w.set_font_size(size);
        w.set_font(f);
        w.set_source(fg);
        if constexpr (vertical == TextAlign::left) {
            w.move_to(text_pos);
        } else if constexpr (vertical == TextAlign::center ||
                             vertical == TextAlign::right) {
            const auto te{w.get_text_extent(s)};
            auto pos{text_pos};
            if constexpr (vertical == TextAlign::center) {
                pos.x = pos.x + (area - m).w / 2 - te.width / 2;
            } else if constexpr (vertical == TextAlign::right) {
                pos.x = pos.x + (area - m).w - te.width;
            }
            w.move_to(pos);
        }
        w.print_text(move(s));
    }
};
} // namespace fprd