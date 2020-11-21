/// @file Window.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <cairo/cairo-xlib.h>
#include <cairo/cairo.h>
#include <fprd/Color.hpp>
#include <fprd/Font.hpp>
#include <fprd/Image.hpp>
#include <fprd/Pattern.hpp>
#include <fprd/Utils.hpp>
#include <fprd/Xlib.hpp>

namespace fprd {
using namespace std;

template <class O>
concept Source = is_same_v<O, Color> || is_base_of_v<Pattern, O>;

/// Is this design pattern bad?
/// I feel like this is like a GOD-class.
class FPRWindow {
    /// Our connection to the X11 server.
    const X11 &x11;
    /// The X11 window we create.
    const Window w;
    /// The surface which we are drawing to.
    cairo_surface_t *const surface;
    /// Context used for draw calls.
    cairo_t *const c;

  public:
    /// Create a new window.
    /// @param x11
    /// @param pos
    /// @param size
    FPRWindow(const X11 &x11, Position<int> pos, Size<unsigned int> size)
        : FPRWindow{x11, x11.default_screen(), pos, size} {}

    /// Change the current source to a color.
    /// @param color
    void set_source(Color color) {
        cairo_set_source_rgba(c, color.r, color.g, color.b, color.a);
    }
    /// Set the current source to a pattern.
    /// @param p
    void set_source(const Pattern &p) { cairo_set_source(c, p); }
    /// Set the current source to an Image.
    void set_source(const Image &i, Position<float> pos) {
        cairo_set_source_surface(c, i, pos.x, pos.y);
    };

    void draw_image(const Image &i, Position<float> pos, Size<float> size) {
        cairo_save(c);
        cairo_translate(c, pos.x, pos.y);
        cairo_scale(c, (double)size.w / i.w, (double)size.h / i.h);
        cairo_set_source_surface(c, i, 0, 0);
        clear();
        cairo_restore(c);
    }

    /// Stroke along the current path.
    void stroke() { cairo_stroke(c); }
    /// Fill the current shape.
    void fill() { cairo_fill(c); }
    /// Paint the entire surface with the current color.
    void clear() { cairo_paint(c); }

    /// Draw a rectangle.
    /// @param pos
    /// @param size
    void rectangle(Position<float> pos, Size<float> size) {
        cairo_rectangle(c, pos.x, pos.y, size.w, size.h);
    }

    /// Move the drawing cursor to a position.
    /// @param pos
    void move_to(Position<float> pos) { cairo_move_to(c, pos.x, pos.y); }

    /// Change the drawing line width.
    /// @param width
    void set_line_width(double width) { cairo_set_line_width(c, width); }
    /// Draw a line to pos from the current cursor position.
    /// @param pos
    void line_to(Position<float> pos) { cairo_line_to(c, pos.x, pos.y); }

    /// Set the font to a certain one.
    /// @param font
    void set_font(const Font &font) { cairo_set_font_face(c, font); }
    /// Set the font size.
    /// @param size
    void set_font_size(double size) { cairo_set_font_size(c, size); };
    /// Print text on the screen.
    /// @param s
    void print_text(string &&s) { cairo_show_text(c, s.c_str()); }

    /// Obtain the font extent.
    /// @param font
    /// @return auto
    auto get_font_extent(const Font &font) {
        cairo_font_extents_t e;
        cairo_font_extents(c, &e);
        return e;
    }
    /// Obtain the text extent.
    /// @param s
    /// @return auto
    auto get_text_extent(const string &s) {
        cairo_text_extents_t e;
        cairo_text_extents(c, s.c_str(), &e);
        return e;
    }

    /// Flush the draw commands.
    void flush() {
        cairo_surface_flush(surface);
        x11.flush();
    }

    /// Cleanup after our selves.
    ~FPRWindow() {
        cairo_destroy(c);
        cairo_surface_destroy(surface);
        x11.destroy_window(w);
    }

  private:
    FPRWindow(const X11 &x11, int screen, Position<int> pos,
              Size<unsigned int> size)
        : x11{x11}, w{[&x11, screen, pos, size]() {
              const auto root{x11.root_window(screen)};
              const auto w{[&x11, root, size]() {
                  XSetWindowAttributes attr{
                      ParentRelative,
                      0,
                      0,
                      0,
                      0,
                      0,
                      Always,
                      0,
                      0,
                      False,
                      ExposureMask | StructureNotifyMask | ButtonPressMask |
                          ButtonReleaseMask,
                      0,
                      False,
                      0,
                      None,
                  };

                  return x11.create_window(
                      root, {0, 0}, size, 0, CopyFromParent, InputOutput,
                      CopyFromParent,
                      CWOverrideRedirect | CWBackingStore | CWBackPixel, attr);
              }()};

              XWMHints hints{
                  .flags = InputHint | StateHint,
                  .input = True,
                  .initial_state = NormalState,
              };
              //   string name{"fprdesktop"};
              //   XClassHint chint{name.data(), name.data()};
              x11.set_WM_properties(w, "", "", {}, nullopt, hints, nullopt);
              x11.set_WM_protocols(w, {});

              x11.change_property(w, x11.atom("_NET_WM_WINDOW_TYPE"), XA_ATOM,
                                  32, PropModeReplace,
                                  array<unsigned long, 1>{
                                      x11.atom("_NET_WM_WINDOW_TYPE_DESKTOP")});

              x11.map_window(w);
              x11.select_input(w, ExposureMask | StructureNotifyMask |
                                      ButtonPressMask);

              x11.move_window(w, pos);
              return w;
          }()},
          surface{cairo_xlib_surface_create(
              x11.display(), w, x11.default_visual(screen), size.w, size.h)},
          c{cairo_create(surface)} {}
};
}; // namespace fprd