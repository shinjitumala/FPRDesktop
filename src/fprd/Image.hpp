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

namespace fprd {
using namespace std;
using namespace filesystem;

class FPRWindow;

class Image {
    friend FPRWindow;

    cairo_surface_t *i;

  public:
    Image(path &&path)
        : i{[&]() {
              if (!exists(path)) {
                  fatal_error("Missing file: " << path);
              }
              return cairo_image_surface_create_from_png(path.c_str());
          }()} {}

    ~Image() { cairo_surface_destroy(i); }

  private:
    operator decltype(i)() const { return i; };
};
}; // namespace fprd