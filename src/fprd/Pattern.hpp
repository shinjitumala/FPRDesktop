/// @file Pattern.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <cairo/cairo.h>
#include <fprd/Color.hpp>
#include <utility>
#include <fprd/Utils.hpp>

namespace fprd {
using namespace std;

class FPRWindow;

/// Abstraction of pattern objects.
class Pattern {
    friend FPRWindow;

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

  protected:
    operator decltype(p)() const { return p; };

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
    PatternLinear(Position start, Position end,
                  initializer_list<pair<double, Color>> stops)
        : Pattern{cairo_pattern_create_linear(start.x, start.y, end.x, end.y)} {
        for (auto [o, c] : stops) {
            cairo_pattern_add_color_stop_rgba(p, o, c.r, c.b, c.b, c.a);
        }
    }
};
} // namespace fprd