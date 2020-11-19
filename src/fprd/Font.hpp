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
    Font(string &&family, cairo_font_slant_t s, cairo_font_weight_t w)
        : f{cairo_toy_font_face_create(family.c_str(), s, w)} {}

    ~Font() { cairo_font_face_destroy(f); }

  private:
    operator decltype(f)() const { return f; };
};
}; // namespace fprd