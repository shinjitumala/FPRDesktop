/// @file MutableText.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Window.hpp>
#include <fprd/wrapper/Cairo.hpp>

namespace fprd {
/// Vertical alignment for text objects.
enum class VerticalAlign : u_char {
    center,
    left,
    right,
};

/// Base class for drawing text.
struct TextBase {
    const cairo::Font *font;
    Position<float> pos;
    Area<float> area;

    template <VerticalAlign V, cairo::source FG = Color>
    void draw_text(Window &w, const FG &fg, string_view s) const {
        const auto font_size{area.h};
        w.set_font_size(font_size);
        w.set_font(*font);
        w.set_source(fg);
        w.move_to([this, &w, font_size, s]() {
            const auto te{w.get_text_extent(s)};
            dbg(if (te.width > area.w || te.height > area.h) {
                dbg_out("WARNING: Text exceeds draw area. Area: "
                        << area << ", Extent: {" << te.width << ", "
                        << te.height << "}");
            });
            const auto height{area.h};
            const auto width{te.x_advance};
            if constexpr (V == VerticalAlign::left) {
                return pos.offset<double>({0, height});
            }
            if constexpr (V == VerticalAlign::center) {
                return pos.offset<double>(
                    {area.w / 2 - width / 2 - te.x_bearing, height});
            }
            if constexpr (V == VerticalAlign::right) {
                return pos.offset<double>({area.w - width, height});
            }
        }());
        w.print_text(s);
    }
};

/// A text that does not move or change color.
/// @tparam V
/// @tparam FG
template <VerticalAlign V, cairo::source FG = Color>
struct Text : public TextBase {
    FG fg;

    void draw(Window &w, string_view text) const {
        draw_text<V, FG>(w, fg, text);
    }
};

/// A text that does not move or change color but gets its area filled with BG
/// every time it is drawn.
/// @tparam V
/// @tparam FG
/// @tparam BG
template <VerticalAlign V, cairo::source FG = Color, cairo::source BG = Color>
struct TextCleared : public TextBase {
    FG fg;
    BG bg;

    void draw(Window &w, string_view text) const {
        w.set_source(bg);
        w.rectangle(pos, area);
        w.fill();
        draw_text<V, FG>(w, fg, text);
    }
};

/// A text must be drawable.
/// @tparam T
template <class T>
concept text = requires(T &t) {
    t.draw(declval<Window &>(), declval<string_view>());
};

/// Utility function.
/// Draw a text only once.
/// @tparam Text
/// @param w
/// @param t
/// @param text
/// @return auto
template <text Text>
auto draw_text_once(Window &w, const Text &t, string_view text) {
    t.draw(w, text);
}
}  // namespace fprd