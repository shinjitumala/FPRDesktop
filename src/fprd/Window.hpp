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
#include <fprd/X11/Utils.hpp>

namespace fprd {
using namespace std;

/// Is this design pattern bad?
/// I feel like this is like a GOD-class.
class FPRWindow {
    /// Our connection to the X11 server.
    Display *const x11;
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
    FPRWindow(Display *x11, pair<int, int> pos, pair<int, int> size)
        : FPRWindow{x11, XDefaultScreen(x11), pos, size} {}

    /// Change the current source to a color.
    /// @param color
    void set_source(Color color) {
        cairo_set_source_rgba(c, color.r, color.g, color.b, color.a);
    }
    /// Set the current source to a pattern.
    /// @param p
    void set_source(const Pattern &p) { cairo_set_source(c, p); }
    /// Set the current source to an Image.
    void set_source(const Image &i, pair<double, double> pos) {
        cairo_set_source_surface(c, i, pos.first, pos.second);
    };

    /// Stroke along the current path.
    void stroke() { cairo_stroke(c); }
    /// Fill the current shape.
    void fill() { cairo_fill(c); }
    /// Paint the entire surface with the current color.
    void clear() { cairo_paint(c); }

    /// Draw a rectangle.
    /// @param pos
    /// @param size
    void rectangle(pair<double, double> pos, pair<double, double> size) {
        cairo_rectangle(c, pos.first, pos.second, size.first, size.second);
    }

    /// Move the drawing cursor to a position.
    /// @param pos
    void move_to(pair<double, double> pos) {
        cairo_move_to(c, pos.first, pos.second);
    }

    /// Change the drawing line width.
    /// @param width
    void set_line_width(double width) { cairo_set_line_width(c, width); }
    /// Draw a line to pos from the current cursor position.
    /// @param pos
    void line_to(pair<double, double> pos) {
        cairo_line_to(c, pos.first, pos.second);
    }

    /// Set the font to a certain one.
    /// @param font
    void set_font(const Font &font) { cairo_set_font_face(c, font); }
    /// Set the font size.
    /// @param size
    void set_font_size(double size) { cairo_set_font_size(c, size); };
    /// Print text on the screen.
    /// @param s
    void print_text(string &&s) { cairo_show_text(c, s.c_str()); }

    /// Flush the draw commands.
    void flush() {
        cairo_surface_flush(surface);
        XFlush(x11);
    }

    /// Cleanup after our selves.
    ~FPRWindow() {
        cairo_destroy(c);
        cairo_surface_destroy(surface);
        XDestroyWindow(x11, w);
    }

  private:
    /// Convenience function for getting atoms.
    /// @param name
    /// @return auto
    auto atom(string &&name) { return XInternAtom(x11, name.data(), False); }

    FPRWindow(Display *x11, int screen, pair<int, int> pos, pair<int, int> size)
        : x11{x11}, w{[x11, screen, pos, size, this]() {
              const auto root{XRootWindow(x11, screen)};
              const auto w{[x11, root, size]() {
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

                  return XCreateWindow(
                      x11, root, 0, 0, size.first, size.second, 0,
                      CopyFromParent, InputOutput, CopyFromParent,
                      CWOverrideRedirect | CWBackingStore | CWBackPixel, &attr);
              }()};

              XWMHints hints{
                  .flags = InputHint | StateHint,
                  .input = True,
                  .initial_state = NormalState,
              };
              string name{"fprdesktop"};
              XClassHint chint{name.data(), name.data()};
              XmbSetWMProperties(x11, w, nullptr, nullptr, {}, 0, nullptr,
                                 &hints, &chint);
              XSetWMProtocols(x11, w, nullptr, 0);
              array<unsigned long, 1> prop{atom("_NET_WM_WINDOW_TYPE_DESKTOP")};
              XChangeProperty(x11, w, atom("_NET_WM_WINDOW_TYPE"), XA_ATOM, 32,
                              PropModeReplace,
                              reinterpret_cast<unsigned char *>(prop.data()),
                              prop.size());

              XMapWindow(x11, w);
              XSelectInput(
                  x11, w, ExposureMask | StructureNotifyMask | ButtonPressMask);

              XMoveWindow(x11, w, pos.first, pos.second);
              return w;
          }()},
          surface{cairo_xlib_surface_create(x11, w, XDefaultVisual(x11, screen),
                                            size.first, size.second)},
          c{cairo_create(surface)} {}
};
}; // namespace fprd