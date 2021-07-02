/// @file Window.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <cairo/cairo.h>
#include <fprd/Theme.hpp>
#include <fprd/util/format.hpp>
#include <fprd/util/ranges.hpp>
#include <fprd/wrapper/Cairo.hpp>
#include <fprd/wrapper/Xlib.hpp>
#include <llvm/ADT/SmallString.h>

namespace fprd {
using namespace std;

/// Some constant values for our window.
namespace window {
/// The size of all fonts.
static constexpr double fontsize{10};
/// letter: The size of a single character.
/// delta: The adjustment for the position of each line.
static auto [csize, line_delta]{[] {
    cairo::Surface surf({});
    surf.set_font(theme::normal);
    surf.set_font_size(fontsize);
    const auto e{surf.text_ext("A")};
    return make_pair(Area{e.width, fontsize}, Position{-0.0, -(fontsize - e.height) / 2});
}()};
}; // namespace window

///
/// @tparam W Width in characters.
/// @tparam H Height in characters.
template <size_t W, size_t H> class Window {
  public:
    class Lines : public array<array<char, W + 1>, H> {
        using Base = array<array<char, W + 1>, H>;

      public:
        using Base::operator[];

        /// Initialized by filling with spaces.
        Lines() {
            for (auto &l : *this) {
                snprintf(l, "%*c", W, ' ');
            }
        }

        template <class... Args> auto printf(size_t line, string_view fmt, Args... args) -> auto {
            return snprintf(operator[](line), fmt, args...);
        }
        template <class... Args>
        auto printf_st(size_t line, size_t s, size_t e, string_view fmt, Args... args) -> auto {
            auto &l{(*this)[line]};
            return snprintf_st({&l[s], &l[e]}, fmt, args...);
        }
    };

  private:
    /// Our connection to the X11 server.
    x11::Connection x11;
    /// The X11 window we create.
    ::Window w;

    /// The buffer surface.
    cairo::Surface canvas;
    /// The surface connected to the X11 window.
    cairo::Surface win;

  public:
    /// Create a new window.
    /// @param x11
    /// @param pos
    /// @param size
    Window(string_view display_name, Position<int> pos)
        : Window{x11::Connection{display_name.data()}, pos, window::csize.scale({W, H})} {}

    Window(const Window &) = delete;

    /// @param s Has to be non-const because using `llvm::SmallVector<S>::c_str()` might modify itself.
    auto update(const Lines &s) -> void {
        multiline_print(s);
        flush();
    }

  private:
    /// @param lines
    auto multiline_print(const Lines &lines) -> void {
        for (const auto [idx, line] : lines | enumerate) {
            canvas.move(Position{0.0, (idx + 1) * window::csize.h} + window::line_delta);
            canvas.set_font(theme::normal);
            canvas.set_font_size(window::fontsize);
            canvas.set_color(theme::white);
            canvas.print_text(line.data());
        }
    }

    /// Flush the draw commands and draw to the x11 window.
    void flush() {
        canvas.flush();

        win.paste(canvas);
        x11.flush();

        canvas.set_color(theme::black);
        canvas.fill_surface();
    }

    /// For clean code.
    /// @param x11
    /// @param pos
    /// @param size
    Window(x11::Connection &&x11, Position<int> pos, Area<unsigned int> size)
        : x11{move(x11)}, w{[&x11 = this->x11, pos, size]() {
              int status;
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
                      ExposureMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask,
                      0,
                      False,
                      0,
                      None,
                  };

                  return x11.create_window(root, {0, 0}, size, 0, CopyFromParent, InputOutput, CopyFromParent,
                                           CWOverrideRedirect | CWBackingStore | CWBackPixel, attr);
              }()};

              status = x11.change_property(w, x11.atom("_NET_WM_WINDOW_TYPE"), XA_ATOM, 32, PropModeReplace,
                                           array<unsigned long, 1>{x11.atom("_NET_WM_WINDOW_TYPE_DESKTOP")});
              if (status == 0) {
                  fatal_error("X11: Failed to change property.");
              }

              status = x11.map_window(w);
              if (status == 0) {
                  fatal_error("X11: Failed to map window.");
              }

              status = x11.move_window(w, pos);
              if (status == 0) {
                  fatal_error("X11: Failed to move window.");
              }
              return w;
          }()},
          canvas{size}, win{this->x11, this->w} {}
};
}; // namespace fprd