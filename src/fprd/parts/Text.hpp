/// @file MutableText.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <cairo/cairo.h>

#include <fprd/Window.hpp>
#include <optional>

namespace fprd {
enum class VerticalAlign : u_char {
    center,
    left,
    right,
};

/// The base data for texts.
struct TextBase {
    const Font *font{nullptr};

    Position<float> pos;
    Area<float> area;
};

/// The detailed text with behavioral changes based on the alignment and
/// foreground source type.
/// @tparam vertical
/// @tparam Foreground
template <VerticalAlign V, Source FG = Color>
struct Text : public TextBase {
    FG fg;

    void update(FPRWindow &w, string_view s) const {
        const auto font_size{area.h * 0.80F};
        w.set_font_size(font_size);
        if (font == nullptr) {
            fatal_error("No fonts set.");
        }
        w.set_font(*font);
        w.set_source(fg);
        w.move_to([this, &w, &s, font_size]() {
            const auto te{w.get_text_extent(s)};
            dbg(if (te.width > area.w || te.height > area.h) {
                dbg_out("WARNING: Text exceeds draw area. Area: "
                        << area << ", Extent: {" << te.width << ", "
                        << te.height << "}");
            });
            if constexpr (V == VerticalAlign::left) {
                return pos.offset({0, font_size});
            }
            if constexpr (V == VerticalAlign::center) {
                return pos.offset({area.w / 2 - te.width / 2, font_size});
            }
            if constexpr (V == VerticalAlign::right) {
                return pos.offset({area.w - te.width, font_size});
            }
        }());
        w.print_text(s);
    }

    template <VerticalAlign OV>
    auto &operator=(const Text<OV, FG> &rhs) {
        *static_cast<TextBase *>(this) = static_cast<const TextBase &>(rhs);
        fg = rhs.fg;
        return *this;
    }
};

/// Fills the background area with color before updating text.
/// @tparam vertical
/// @tparam Foreground
/// @tparam Background
template <VerticalAlign V, Source FG = Color, Source BG = Color>
struct DynamicText : public Text<V, FG> {
   private:
    using Base = Text<V, FG>;

   public:
    BG bg;

    template <VerticalAlign OV>
    auto &operator=(const DynamicText<OV, FG, BG> &rhs) {
        *static_cast<Text<V, FG> *>(this) =
            static_cast<const Text<OV, FG> &>(rhs);
        bg = rhs.bg;
        return *this;
    }

    void update(FPRWindow &w, string_view s) const {
        w.set_source(bg);
        w.rectangle(Base::pos, Base::area);
        w.fill();
        Base::update(w, s);
    }
};

template <VerticalAlign V, Source FG = Color>
using StaticText = Text<V, FG>;
}  // namespace fprd