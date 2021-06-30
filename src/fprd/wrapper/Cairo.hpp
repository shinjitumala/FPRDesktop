/// @file cairo.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <X11/X.h>
#include <cairo/cairo-xlib.h>
#include <cairo/cairo.h>

#include <dbg/Log.hpp>
#include <fprd/Types.hpp>
#include <fprd/wrapper/Xlib.hpp>

namespace fprd {
using namespace ::std;
using namespace ::std::filesystem;

/// Some wrapped stuff for modernization.
namespace cairo {

/// Self-cleaning cairo_font_face_t.
/// FIXME: Google sanitizers seem to detect some leaks. idk why lol.
class Font {
    /// Wrapped thing.
    cairo_font_face_t *f;

  public:
    /// Default constructor.
    Font() : f{nullptr} {};

    /// Create a font.
    /// @param family
    /// @param s
    /// @param w
    Font(string_view family, cairo_font_slant_t s, cairo_font_weight_t w)
        : f{cairo_toy_font_face_create(family.data(), s, w)} {}

    /// Destructor.
    ~Font() { cairo_font_face_destroy(f); }

    /// Copying is disallowed.
    Font(const Font &) = delete;

    /// Explicit casting is allowed.
    /// @return decltype(f)
    explicit operator decltype(f)() const { return f; };
};

/// Modern interface for the good old cairo library.
class Surface {
    cairo_surface_t *surf;
    cairo_t *ctx;

  public:
    /// Create an empty surface with size.
    /// @param a
    Surface(const Area<int> a)
        : surf{cairo_image_surface_create(CAIRO_FORMAT_ARGB32, a.w, a.h)}, ctx{cairo_create(surf)} {}

    /// Create an surface on a X11 window.
    /// The size is set to be the exact same as the window.
    /// @param x11
    /// @param w
    Surface(const x11::Connection &x11, const ::Window &w)
        : surf{[&] {
              auto [status, attr]{x11.get_window_attributes(w)};
              if (!status) {
                  fatal_error("Failed to get window attributes.");
              }

              return cairo_xlib_surface_create(x11.display(), static_cast<::Window>(w),
                                               x11.default_visual(x11.default_screen()), attr.width, attr.height);
          }()},
          ctx{cairo_create(surf)} {}

    /// No copy because it breaks the invariant.
    Surface(const Surface &) = delete;

    /// Change the current source to a color.
    /// @param color
    void set_color(Color color) { cairo_set_source_rgba(ctx, color.r, color.g, color.b, color.a); }
    /// Draw from surface to surface.
    /// WARNING: The surfaces are assumed to have the same size!
    /// @param surf
    void paste(const Surface &surf) {
        cairo_set_source_surface(ctx, surf.surf, 0, 0);
        cairo_paint(ctx);
    }

    /// Paint the entire surface with the current color.
    void fill_surface() { cairo_paint(ctx); }

    /// Move the drawing cursor to a position.
    /// @param pos
    void move(Position<float> pos) { cairo_move_to(ctx, pos.x, pos.y); }

    /// Set the font to a certain one.
    /// @param font
    void set_font(const Font &font) { cairo_set_font_face(ctx, static_cast<cairo_font_face_t *>(font)); }
    /// Set the font size.
    /// @param size
    void set_font_size(double size) { cairo_set_font_size(ctx, size); };
    /// Print text on the screen.
    /// @param s
    void print_text(string_view s) { cairo_show_text(ctx, s.data()); }
    /// Print text on the screen.
    /// @param s
    void print_text(const char *s) { cairo_show_text(ctx, s); }

    /// Obtain the font extent.
    /// @param font
    /// @return auto
    auto font_ext(const Font &font) {
        cairo_font_extents_t e;
        cairo_font_extents(ctx, &e);
        return e;
    }
    auto text_ext(string_view s) {
        cairo_text_extents_t e;
        cairo_text_extents(ctx, s.data(), &e);
        return e;
    }

    /// Execute the pending draw calls.
    /// @return auto
    auto flush() { cairo_surface_flush(surf); }

    /// Destructor.
    ~Surface() {
        cairo_destroy(ctx);
        cairo_surface_destroy(surf);
    }
};
}; // namespace cairo
}; // namespace fprd