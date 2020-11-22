/// @file Image.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <cairo/cairo.h>

#include <dbg/Log.hpp>
#include <filesystem>
#include <fprd/Color.hpp>

namespace fprd {
using namespace std;
using namespace filesystem;

class FPRWindow;

class Image {
    friend FPRWindow;

    cairo_surface_t *i;

   public:
    int w;
    int h;

    Image(path &&path)
        : i{[&]() {
              if (!exists(path)) {
                  fatal_error("Missing file: " << path);
              }
              return cairo_image_surface_create_from_png(path.c_str());
          }()},
          w{cairo_image_surface_get_width(i)},
          h{cairo_image_surface_get_height(i)} {}
    Image(path &&path, Color mask) : Image{move(path)} {
        auto *const c{cairo_create(i)};
        cairo_set_operator(c, CAIRO_OPERATOR_ATOP);
        cairo_set_source_rgba(c, mask.r, mask.g, mask.b, mask.a);
        cairo_paint(c);
        cairo_destroy(c);
    }

    ~Image() { cairo_surface_destroy(i); }

   private:
    operator decltype(i)() const { return i; };
};
};  // namespace fprd