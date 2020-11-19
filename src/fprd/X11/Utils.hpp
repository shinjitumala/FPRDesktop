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
#include <span>

namespace fprd {
using namespace std;

/// 'XFree()' is called on the span upon distruction.
/// @tparam V
template <class V> struct FreedSpan : public span<V> {
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

/// @param d Specifies the connection to the X server.
/// @param w Specifies the window whose property you want to obtain.
/// @param property Specifies the property name.
/// @param offset Specifies the offset in the specified property (in 32-bit
/// quantities) where the data is to be retrieved.
/// @param length Specifies the length in 32-bit multiples of the data to be
/// retrieved.
/// @param delete_after Specifies a Boolean value that determines whether the
/// property is deleted.
/// @param request_type Specifies the atom identifier associated with the
/// property type or AnyPropertyType.
/// @return auto
/// {
///     Returns the atom identifier that defines the actual type of the
///     property.
///     Returns the actual format of the property.
///     Returns the actual number of 8-bit, 16-bit, or 32-bit items stored in
///     the prop_return data.
///     Returns the number of bytes remaining to be read in the property if a
///     partial read was performed.
///     Returns the data in the specified format.
/// }
auto XGetWindowProperty(Display *d, Window w, Atom property, long offset,
                        long length, bool delete_after, Atom request_type) {
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

auto XQueryTree(Display *d, Window w) {
    Window root;
    Window parent;
    Window *children;
    unsigned int children_count;
    if (XQueryTree(d, w, &root, &parent, &children, &children_count) == 0) {
        fatal_error("'XQueryTree' failed!");
    }
    return make_tuple(root, parent,
                      FreedSpan<Window>{children, children + children_count});
}

auto XGetWindowAttributes(Display *d, Window w) {
    XWindowAttributes attr;
    const auto status{XGetWindowAttributes(d, w, &attr)};
    return make_pair(status != 0, attr);
}

}; // namespace fprd