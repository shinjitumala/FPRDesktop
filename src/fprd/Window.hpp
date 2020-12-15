/// @file Window.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Cairo.hpp>
#include <fprd/Color.hpp>
#include <fprd/Theme.hpp>
#include <fprd/Utils.hpp>
#include <fprd/Xlib.hpp>

namespace fprd {
using namespace std;

template <class O>
concept Source = is_same_v<O, Color> || is_base_of_v<cairo::Pattern, O>;

/// Is this design pattern bad?
/// I feel like this is like a GOD-class.
class Window {
    /// Our connection to the X11 server.
    const X11 x11;
    /// The X11 window we create.
    const ::Window w;

    /// We draw to this surface.
    cairo::Surface canvas;
    /// The buffer surface.
    cairo::Surface buf;
    /// The surface connected to the X11 window.
    cairo::Surface win;

   public:
    /// Create a new window.
    /// @param x11
    /// @param pos
    /// @param size
    Window(string_view display_name, Position<int> pos, Area<unsigned int> size)
        : x11{display_name.data()},
          w{[&x11 = this->x11, pos, size]() {
              const auto root{x11.root_window(x11.default_screen())};
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

              x11.change_property(w, x11.atom("_NET_WM_WINDOW_TYPE"), XA_ATOM,
                                  32, PropModeReplace,
                                  array<unsigned long, 1>{
                                      x11.atom("_NET_WM_WINDOW_TYPE_DESKTOP")});

              x11.map_window(w);

              x11.move_window(w, pos);
              return w;
          }()},
          canvas{size},
          buf{size},
          win{x11, w} {}

    /// Flush the draw commands.
    void flush() {
        canvas.flush();

        buf.set_source(theme::black);
        buf.paint();
        buf.draw(canvas);
        buf.flush();

        win.draw(buf);

        x11.flush();
    }

    /// Arrow invokes the canvas.
    /// @return auto
    auto operator->() { return &canvas; }

    /// Cleanup after our selves.
    ~Window() { x11.destroy_window(w); }
};
};  // namespace fprd