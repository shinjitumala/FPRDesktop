/// @file Window.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Theme.hpp>
#include <fprd/wrapper/Cairo.hpp>
#include <fprd/wrapper/Xlib.hpp>

namespace fprd {
using namespace std;

/// Is this design pattern bad?
/// I feel like this is like a GOD-class.
class Window : public cairo::Surface {
    using Base = cairo::Surface;

    /// Our connection to the X11 server.
    x11::Connection x11;
    /// The X11 window we create.
    x11::Window w;

    /// The buffer surface.
    cairo::Surface buf;
    /// The surface connected to the X11 window.
    cairo::Surface win;

    /// The base flush function should be private.
    using Base::flush;

   public:
    unsigned char frame_counter;

    /// Create a new window.
    /// @param x11
    /// @param pos
    /// @param size
    Window(string_view display_name, Position<int> pos, Area<unsigned int> size)
        : Window{x11::Connection{display_name.data()}, pos, size} {}

    /// Flush the draw commands and draw to the x11 window.
    void flush() {
        Base::flush();

        buf.set_source(theme::black);
        buf.paint();
        buf.draw(*this);
        buf.flush();

        win.draw(buf);

        x11.flush();
    }

    Window(const Window&) = delete;
    Window(Window&&) = default;

   private:
    /// For clean code.
    /// @param x11
    /// @param pos
    /// @param size
    Window(x11::Connection&& x11, Position<int> pos, Area<unsigned int> size)
        : cairo::Surface{size},
          x11{move(x11)},
          w{[&x11 = this->x11, pos, size]() {
              /// Obtain the correct root window and create a new window.
              /// FIXME: The window disappears when I click on the
              /// desktop LOL.
              const auto root{x11.root_window(x11.default_screen())};
              auto w{[&x11, root, size]() {
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

              x11.change_property(w, x11.atom("_NET_WM_WINDOW_TYPE"), XA_ATOM,
                                  32, PropModeReplace,
                                  array<unsigned long, 1>{
                                      x11.atom("_NET_WM_WINDOW_TYPE_DESKTOP")});

              x11.map_window(w);

              x11.move_window(w, pos);
              return w;
          }()},
          buf{size},
          win{this->x11, this->w} {}
};
};  // namespace fprd