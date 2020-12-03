/// @file Font.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <cairo/cairo.h>

#include <string>

namespace fprd {
using namespace std;
class FPRWindow;

class Font {
    friend FPRWindow;

    cairo_font_face_t *f;

   public:
    Font() : f{nullptr} {};

    Font(string_view family, cairo_font_slant_t s, cairo_font_weight_t w)
        : f{cairo_toy_font_face_create(family.data(), s, w)} {}

    ~Font() { destroy(); }

    Font &operator=(Font &&rhs) noexcept {
        destroy();
        f = rhs.f;
        rhs.f = nullptr;
        return *this;
    };
    Font(Font &&f) noexcept : Font() { *this = move(f); };

   private:
    operator decltype(f)() const { return f; };

    void destroy() {
        if (f != nullptr) {
            cairo_font_face_destroy(f);
        }
    }
};
};  // namespace fprd