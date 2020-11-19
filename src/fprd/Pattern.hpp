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

namespace fprd {
using namespace std;

class FPRWindow;

/// Abstraction of pattern objects.
class Pattern {
    friend FPRWindow;

  protected:
    cairo_pattern_t *p;

    /// Initialize the pattern.
    Pattern(decltype(p) p) : p{p} {}

    /// Cleanup patterns in the end.
    ~Pattern() { cairo_pattern_destroy(p); }

    operator decltype(p)() const { return p; };
};

/// Linear pattern
struct PatternLinear : public Pattern {
    /// @param start
    /// @param end
    /// @param stops List of pairs of offset and color.
    PatternLinear(pair<double, double> start, pair<double, double> end,
                  initializer_list<pair<double, Color>> stops)
        : Pattern{cairo_pattern_create_linear(start.first, start.second,
                                              end.first, end.second)} {
        for (auto [o, c] : stops) {
            cairo_pattern_add_color_stop_rgba(p, o, c.r, c.b, c.b, c.a);
        }
    }
};
} // namespace fprd