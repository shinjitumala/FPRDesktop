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
#include <fprd/Xlib.hpp>

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
    ~Font() { destroy(); }

    /// Move assignment is allowed.
    /// @param rhs
    /// @return Font&
    Font &operator=(Font &&rhs) noexcept {
        destroy();
        f = rhs.f;
        rhs.f = nullptr;
        return *this;
    };
    /// Moving is allowed
    /// @param f
    Font(Font &&f) noexcept : Font() { *this = move(f); };

    /// Copying is disallowed.
    Font(const Font &) = delete;

    /// Explicit casting is allowed.
    /// @return decltype(f)
    explicit operator decltype(f)() const { return f; };

   private:
    /// Private destruction used for robust implementation of move stuff.
    void destroy() {
        if (f != nullptr) {
            cairo_font_face_destroy(f);
        }
    }
};

/// Abstraction for PNG images because we use it a lot in fprd.
class Image {
    /// Wrapped thing.
    cairo_surface_t *i;

   public:
    /// The size of the png file.
    Area<int> size;

    /// Load image
    /// @param path
    Image(const path path)
        : i{[&]() {
              if (!exists(path)) {
                  fatal_error("Missing file: " << path);
              }
              return cairo_image_surface_create_from_png(path.c_str());
          }()},
          size{cairo_image_surface_get_width(i),
               cairo_image_surface_get_height(i)} {}

    /// Load image with color mask
    /// @param path
    /// @param mask
    Image(const path path, const Color mask) : Image{path} {
        auto *const c{cairo_create(i)};
        cairo_set_operator(c, CAIRO_OPERATOR_ATOP);
        cairo_set_source_rgba(c, mask.r, mask.g, mask.b, mask.a);
        cairo_paint(c);
        cairo_destroy(c);
    }

    /// Destructor.
    ~Image() { cairo_surface_destroy(i); }

    /// No copies.
    Image(const Image &) = delete;
    /// Moving is okay.
    Image(Image &&) = default;

    /// Allow explicit casts.
    /// @return decltype(i)
    explicit operator decltype(i)() const { return i; };
};

/// Abstraction of pattern objects.
class Pattern {
   protected:
    cairo_pattern_t *p;

   public:
    Pattern() : p{nullptr} {};

    /// Initialize the pattern.
    Pattern(decltype(p) p) : p{p} {}
    Pattern &operator=(Pattern &&rhs) noexcept {
        destroy();
        p = rhs.p;
        rhs.p = nullptr;
        return *this;
    }
    Pattern(Pattern &&p) noexcept : Pattern() { *this = move(p); }

    /// Cleanup patterns in the end.
    ~Pattern() { destroy(); }

    /// No copy.
    Pattern(const Pattern &) = delete;

    /// Allow explicit casts.
    /// @return decltype(p)
    operator decltype(p)() const { return p; };

   private:
    void destroy() {
        if (p != nullptr) {
            cairo_pattern_destroy(p);
        }
    };
};

/// Linear pattern
struct PatternLinear : public Pattern {
    /// @param start
    /// @param end
    /// @param stops List of pairs of offset and color.
    PatternLinear(Position<float> start, Position<float> end,
                  initializer_list<pair<float, Color>> stops)
        : Pattern{cairo_pattern_create_linear(start.x, start.y, end.x, end.y)} {
        for (auto [o, c] : stops) {
            cairo_pattern_add_color_stop_rgba(p, o, c.r, c.b, c.b, c.a);
        }
    }
};

/// Modern interface for the good old cairo library.
class Surface {
    cairo_surface_t *const surf;
    cairo_t *const ctx;

   public:
    /// Create an empty surface with size.
    /// @param a
    Surface(const Area<int> a)
        : Surface{cairo_image_surface_create(CAIRO_FORMAT_ARGB32, a.w, a.h)} {}

    /// Create an surface on a X11 window.
    /// The size is set to be the exact same as the window.
    /// @param x11
    /// @param w
    Surface(const X11 &x11, Window w)
        : Surface{[&] {
              auto [status, attr]{x11.get_window_attributes(w)};
              if (!status) {
                  fatal_error("Failed to get window attributes.");
              }

              return cairo_xlib_surface_create(
                  x11.display(), w, x11.default_visual(x11.default_screen()),
                  attr.width, attr.height);
          }()} {}

    /// No copy because it breaks the invariant.
    Surface(const Surface &) = delete;
    /// Moving is okay though.
    Surface(Surface &&) = default;

    /// Change the current source to a color.
    /// @param color
    void set_source(Color color) {
        cairo_set_source_rgba(ctx, color.r, color.g, color.b, color.a);
    }
    /// Set the current source to a pattern.
    /// @param p
    void set_source(const Pattern &p) { cairo_set_source(ctx, p); }
    /// Set the current source to an Image.
    void set_source(const Image &i, Position<float> pos) {
        cairo_set_source_surface(ctx, static_cast<cairo_surface_t *>(i), pos.x,
                                 pos.y);
    };

    /// Convenience function for drawing images into surfaces.
    /// @param i
    /// @param pos
    /// @param size
    void draw(const Image &i, Position<float> pos, Area<float> size) {
        cairo_save(ctx);
        cairo_translate(ctx, pos.x, pos.y);
        cairo_scale(ctx, (double)size.w / i.size.w, (double)size.h / i.size.h);
        cairo_set_source_surface(ctx, static_cast<cairo_surface_t *>(i), 0, 0);
        paint();
        cairo_restore(ctx);
    }

    /// Draw from surface to surface.
    /// WARNING: The surfaces are assumed to have the same size!
    /// @param surf
    void draw(const Surface &surf) {
        cairo_set_source_surface(ctx, surf.surf, 0, 0);
        cairo_paint(ctx);
    }

    /// Stroke along the current path.
    void stroke() { cairo_stroke(ctx); }
    /// Fill the current shape.
    void fill() { cairo_fill(ctx); }
    /// Paint the entire surface with the current color.
    void paint() { cairo_paint(ctx); }

    /// Draw a rectangle.
    /// @param pos
    /// @param size
    void rectangle(Position<float> pos, Area<float> size) {
        cairo_rectangle(ctx, pos.x, pos.y, size.w, size.h);
    }

    /// Move the drawing cursor to a position.
    /// @param pos
    void move_to(Position<float> pos) { cairo_move_to(ctx, pos.x, pos.y); }

    /// Change the drawing line width.
    /// @param width
    void set_line_width(double width) { cairo_set_line_width(ctx, width); }
    /// Draw a line to pos from the current cursor position.
    /// @param pos
    void line_to(Position<float> pos) { cairo_line_to(ctx, pos.x, pos.y); }
    /// Try to close the current path.
    void close_path() { cairo_close_path(ctx); };
    /// Create an arc line clock-wise.
    /// @param center
    /// @param radious
    /// @param start Start angle in radians.
    /// @param end  End angle in radians.
    void arc(Position<float> center, float radious, float start, float end) {
        cairo_arc(ctx, center.x, center.y, radious, start, end);
    }
    /// Create an arc line counter clock-wise.
    /// @param center
    /// @param radious
    /// @param start Start angle in radians.
    /// @param end  End angle in radians.
    void rarc(Position<float> center, float radious, float start, float end) {
        cairo_arc_negative(ctx, center.x, center.y, radious, start, end);
    }

    /// Set the font to a certain one.
    /// @param font
    void set_font(const Font &font) {
        cairo_set_font_face(ctx, static_cast<cairo_font_face_t *>(font));
    }
    /// Set the font size.
    /// @param size
    void set_font_size(double size) { cairo_set_font_size(ctx, size); };
    /// Print text on the screen.
    /// @param s
    void print_text(string_view s) { cairo_show_text(ctx, s.data()); }

    /// Obtain the font extent.
    /// @param font
    /// @return auto
    auto get_font_extent(const Font &font) {
        cairo_font_extents_t e;
        cairo_font_extents(ctx, &e);
        return e;
    }
    /// Obtain the text extent.
    /// @param s
    /// @return auto
    auto get_text_extent(string_view s) {
        cairo_text_extents_t e;
        cairo_text_extents(ctx, s.data(), &e);
        return e;
    }

    /// Execute the pending draw calls.
    /// @return auto 
    auto flush(){
        cairo_surface_flush(surf);
    }

    /// Destructor.
    ~Surface() {
        cairo_destroy(ctx);
        cairo_surface_destroy(surf);
    }

   private:
    /// To hell with redundancy!
    /// @param surf
    Surface(cairo_surface_t *const surf)
        : surf{surf}, ctx{cairo_create(surf)} {}
};
};  // namespace cairo
};  // namespace fprd