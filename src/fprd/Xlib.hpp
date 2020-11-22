/// @file Utils.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <dbg/Log.hpp>
#include <experimental/source_location>
#include <fprd/Utils.hpp>
#include <span>

namespace fprd {
using namespace std;

/// 'XFree()' is called on the span upon distruction.
/// @tparam V
template <class V>
struct FreedSpan : public span<V> {
   private:
    /// Convenient alias.
    using Base = span<V>;

   public:
    /// Use the base constructors.
    using Base::Base;

    /// @param f
    FreedSpan(FreedSpan &&f) noexcept : Base{move(f)} {
        // Empty the moved span to avoid double frees.
        static_cast<Base &>(f) = {};
    }
    /// Disallow copying because it breaks the invariant of this class.
    FreedSpan(const FreedSpan &) = delete;

    /// Free if we own any data.
    ~FreedSpan() {
        if (!Base::empty()) {
            XFree(Base::data());
        }
    }
};

/// A connection to a X11 server.
/// A GOD-class? Probably bad.
class X11 {
    /// The real type.
    Display *d;

   public:
    /// Open a connection to the X11 server.
    /// @param display
    X11(string &&display) : d{XOpenDisplay(display.c_str())} {
        if (d == nullptr) {
            fatal_error("Failed to open display");
        }
    }

    /// We are not allowed to make copies because it breaks the calss's
    /// invariant.
    X11(const X11 &) = delete;
    /// However, we are still allowed to move things.
    /// @param rhs
    /// @return X11&
    X11 &operator=(X11 &&rhs) noexcept {
        d = rhs.d;
        rhs.d = nullptr;
        return *this;
    }
    /// Move constructor.
    /// @param x11
    X11(X11 &&x11) noexcept { *this = move(x11); };

    /// Automatically close the connection.
    ~X11() {
        if (d == nullptr) {
            return;
        };
        XCloseDisplay(d);
    }

    [[nodiscard]] auto display() const { return d; };

    /// @param d Specifies the connection to the X server.
    /// @param w Specifies the window whose property you want to obtain.
    /// @param property Specifies the property name.
    /// @param offset Specifies the offset in the specified property (in 32-bit
    /// quantities) where the data is to be retrieved.
    /// @param length Specifies the length in 32-bit multiples of the data to be
    /// retrieved.
    /// @param delete_after Specifies a Boolean value that determines whether
    /// the property is deleted.
    /// @param request_type Specifies the atom identifier associated with the
    /// property type or AnyPropertyType.
    /// @return auto
    /// {
    ///     Returns the atom identifier that defines the actual type of the
    ///     property.
    ///     Returns the actual format of the property.
    ///     Returns the actual number of 8-bit, 16-bit, or 32-bit items stored
    ///     in the prop_return data. Returns the number of bytes remaining to be
    ///     read in the property if a partial read was performed. Returns the
    ///     data in the specified format.
    /// }
    [[nodiscard]] auto get_window_property(Window w, Atom property, long offset,
                                           long length, bool delete_after,
                                           Atom request_type) const {
        Atom type;
        int format;
        unsigned long nitems;
        unsigned long bytes_after;
        unsigned char *prop;
        XGetWindowProperty(d, w, property, offset, length,
                           static_cast<int>(delete_after), request_type, &type,
                           &format, &nitems, &bytes_after, &prop);
        return make_tuple(
            type, format,
            FreedSpan<unsigned char>{prop, prop + nitems + bytes_after});
    }

    [[nodiscard]] auto query_tree(Window w) const {
        Window root;
        Window parent;
        Window *children;
        unsigned int children_count;
        if (XQueryTree(d, w, &root, &parent, &children, &children_count) == 0) {
            fatal_error("'XQueryTree' failed!");
        }
        return make_tuple(
            root, parent,
            FreedSpan<Window>{children, children + children_count});
    }

    [[nodiscard]] auto get_window_attributes(Window w) const {
        XWindowAttributes attr;
        const auto status{XGetWindowAttributes(d, w, &attr)};
        return make_pair(status != 0, attr);
    }

    auto flush() const { XFlush(d); }

    auto destroy_window(Window w) const { XDestroyWindow(d, w); };
    [[nodiscard]] auto create_window(
        Window parent, Position<int> pos, Size<unsigned int> size,
        unsigned int border_w, int depth, unsigned int window_class,
        Visual *visual, unsigned long value_mask,
        const XSetWindowAttributes &attributes) const {
        return XCreateWindow(d, parent, pos.x, pos.y, size.w, size.h, border_w,
                             depth, window_class, visual, value_mask,
                             const_cast<XSetWindowAttributes *>(&attributes));
    }

    [[nodiscard]] auto root_window(int screen) const {
        return XRootWindow(d, screen);
    }
    [[nodiscard]] auto default_screen() const { return XDefaultScreen(d); };
    [[nodiscard]] auto default_visual(int screen) const {
        return XDefaultVisual(d, screen);
    };

    auto set_WM_properties(Window w, string &&window_name, string &&icon_name,
                           vector<string> args, optional<XSizeHints> &&sh,
                           optional<XWMHints> &&wmh,
                           optional<XClassHint> &&ch) const {
        XmbSetWMProperties(
            d, w, window_name.c_str(), icon_name.c_str(),
            (!args.empty()) ? reinterpret_cast<char **>(args.data()) : nullptr,
            args.size(), (sh) ? &*sh : nullptr, (wmh) ? &*wmh : nullptr,
            (ch) ? &*ch : nullptr);
    }

    [[nodiscard]] auto set_WM_protocols(Window w,
                                        vector<Atom> protocols) const {
        return XSetWMProtocols(d, w,
                               (protocols.empty()) ? nullptr : protocols.data(),
                               protocols.size());
    }

    auto atom(string &&name) const {
        return XInternAtom(d, name.data(), False);
    };

    template <class Element, size_t size>
    [[nodiscard]] auto change_property(Window w, Atom property, Atom type,
                                       int format, int mode,
                                       array<Element, size> data) const {
        return XChangeProperty(d, w, property, type, format, mode,
                               reinterpret_cast<unsigned char *>(data.data()),
                               size);
    }

    [[nodiscard]] auto map_window(Window w) const { return XMapWindow(d, w); }
    [[nodiscard]] auto select_input(Window w, long mask) const {
        return XSelectInput(d, w, mask);
    }

    [[nodiscard]] auto move_window(Window w, Position<int> pos) const {
        return XMoveWindow(d, w, pos.x, pos.y);
    }
};

/// For easy debugging.
/// @param os
/// @param attr
/// @return ostream&
ostream &operator<<(ostream &os, const XWindowAttributes &attr) {
    os << "{" << nl;
    {
        dbg::IndentGuard ig{};
        os << "position: " << attr.x << ", " << attr.y << "," << nl;
        os << "size: " << attr.width << "x" << attr.height << "," << nl;
        os << "border_width: " << attr.border_width << "," << nl;
        os << "depth: " << attr.depth << "," << nl;
        os << "root: " << attr.root << "," << nl;
        os << "screen: " << attr.screen->width << "x" << attr.screen->height
           << "," << nl;
    }
    os << "}";
    return os;
}

};  // namespace fprd