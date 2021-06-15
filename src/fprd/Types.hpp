/// @file Types.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <concepts>
#include <dbg/Log.hpp>
#include <ostream>
#include <ratio>
#include <sstream>

namespace fprd {
using namespace ::std;

/// Types accepted by our utility types.
/// @tparam I
template <class I>
concept number = integral<I> || floating_point<I>;

/// constexpr version of round.
/// Seriously, why the fucks are these still not constexpr?
/// @tparam I
/// @tparam F
/// @param f
/// @return constexpr I
template <integral I, floating_point F>
constexpr I round(F f) {
    auto ipart{static_cast<I>(f)};
    auto fpart{f - static_cast<F>(ipart)};
    if (fpart < -0.5) {
        return ipart - 1;
    }
    if (0.5 <= fpart) {
        return ipart + 1;
    }
    return ipart;
}

/// Our abstraction for margins.
/// @tparam I
template <number I>
struct Margin {
    /// left
    I l;
    /// right
    I r;
    /// top
    I t;
    /// bottom
    I b;

    /// Default constructor.
    constexpr Margin() = default;
    /// Initialize from all four values.
    /// @param l
    /// @param r
    /// @param t
    /// @param b
    constexpr Margin(I l, I r, I t, I b) : l{l}, r{r}, t{t}, b{b} {}
    /// Initialize with equal margins for each dimension.
    /// @param x
    /// @param y
    constexpr Margin(I x, I y) : l{x}, r{y}, t{y}, b{y} {}

    /// Combine margins, summing up the margins in each direction.
    /// @param rhs
    /// @return constexpr Margin
    constexpr Margin operator+(Margin rhs) const {
        return {l + rhs.l, r + rhs.r, t + rhs.t, b + rhs.b};
    }

    /// Scale the margin in each direction.
    /// @tparam S
    /// @param scale
    /// @return requires constexpr
    template <number S>
    requires convertible_to<S, I>
    constexpr Margin operator*(S scale) const {
        I s{static_cast<I>(scale)};
        return {l * s, r * s, t * s, b * s};
    };

    /// Implicit conversion between Margin types.
    /// @tparam S
    /// @param scale
    /// @return requires constexpr
    template <number S>
    requires convertible_to<I, S>
    constexpr operator Margin<S>() const {
        if constexpr (is_floating_point_v<I> && is_integral_v<S>) {
            return {round<S>(l), round<S>(r), round<S>(t), round<S>(b)};
        }
        return {static_cast<S>(l), static_cast<S>(r), static_cast<S>(t),
                static_cast<S>(b)};
    }
};

template <number I>
struct Area;

/// An abstraction for a 2D position.
/// @tparam I
template <number I>
struct Position {
    /// X coordinates.
    I x;
    /// Y coordinates.
    I y;

    /// Default constructor.
    constexpr Position() = default;
    /// Constructor
    /// @param x
    /// @param y
    constexpr Position(I x, I y) : x{x}, y{y} {}

    /// IMPORTANT: Always pad first before applying area adjustments.
    /// @param m
    /// @return constexpr Position
    [[nodiscard]] constexpr Position pad(Margin<I> m) const {
        return {x + m.l, y + m.t};
    }

    /// Add positions.
    /// @param rhs
    /// @return constexpr Position
    constexpr Position operator+(Position rhs) const {
        return {x + rhs.x, y + rhs.y};
    }

    /// Obtain the position after stacking Area 'a' to the right.
    /// @param a
    /// @return constexpr Position
    constexpr Position stack_right(Area<I> a) const { return {x + a.w, y}; }
    /// Obtain the position after stacking Area 'a' under.
    /// @param a
    /// @return constexpr Position
    constexpr Position stack_bottom(Area<I> a) const { return {x, y + a.h}; }
    /// Obtain the position after stacking Area 'a' to the left.
    /// @param a
    /// @return constexpr Position
    constexpr Position stack_left(Area<I> a) const { return {x - a.w, y}; }
    /// Obtain the position after stacking Area 'a' above.
    /// @param a
    /// @return constexpr Position
    constexpr Position stack_top(Area<I> a) const { return {x, y - a.h}; }
    /// Obtain the position after stacking Area 'a' to both right and down.
    /// @param a
    /// @return constexpr Position
    constexpr Position stack(Area<I> a) const { return {x + a.w, y + a.h}; };

    /// Becase we can't type deduct with 'operator+'?
    /// @param rhs
    /// @return constexpr Position
    template <number S = I>
    constexpr Position offset(Position<S> rhs) const {
        return *this + rhs;
    }

    /// Scale the area.
    /// @param scale
    /// @return constexpr Position
    constexpr Position scale(pair<float, float> scale) const {
        return {x * scale.first, y * scale.second};
    }

    /// Implicit conversions between Position types.
    /// @tparam S
    /// @return Position<S>
    template <number S>
    requires convertible_to<I, S>
    constexpr operator Position<S>() const {
        if constexpr (is_floating_point_v<I> && is_integral_v<S>) {
            return {round<S>(x), round<S>(y)};
        }
        return {static_cast<S>(x), static_cast<S>(y)};
    }

    /// Print in a json-like format.
    /// @param os
    /// @return ostream&
    ostream& print(ostream& os) const {
        os << "{" << x << ", " << y << "}";
        return os;
    }
};

/// Represents a 2D area.
/// @tparam I
template <number I>
struct Area {
    /// Width
    I w;
    /// Height
    I h;

    /// Default constructor.
    constexpr Area() = default;
    /// Constructor.
    /// @param w
    /// @param h
    constexpr Area(I w, I h) : w{w}, h{h} {}

    /// Add padding.
    /// @param m
    /// @return constexpr Area
    [[nodiscard]] constexpr Area pad(Margin<I> m) const {
        return {w - m.l - m.r, h - m.t - m.b};
    };
    /// Scale the area.
    /// @param scale
    /// @return constexpr Area
    [[nodiscard]] constexpr Area scale(pair<float, float> scale) const {
        return {w * scale.first, h * scale.second};
    }
    /// Implicit conversions between instances.
    /// @tparam S
    /// @return Area<S>
    template <number S>
    requires convertible_to<I, S>
    constexpr operator Area<S>() const {
        if constexpr (is_floating_point_v<I> && is_integral_v<S>) {
            return {round<S>(w), round<S>(h)};
        }
        return {static_cast<S>(w), static_cast<S>(h)};
    }

    /// Obtains the Position to draw this area so that 'pos' is the bottom left
    /// corner.
    /// @param pos
    /// @return constexpr Position<I>
    [[nodiscard]] constexpr Position<I> bottom_left(Position<I> pos) const {
        return {pos.x, pos.y - h};
    }

    /// Obtains the Position to draw this area so that 'pos' is the top right
    /// corner.
    /// @param pos
    /// @return constexpr Position<I>
    [[nodiscard]] constexpr Position<I> top_right(Position<I> pos) const {
        return {pos.x - w, pos.y};
    }
    /// Obtains the Position to draw this area so that 'pos' is the bottom_right
    /// corner.
    /// @param pos
    /// @return constexpr Position<I>
    [[nodiscard]] constexpr Position<I> bottom_right(Position<I> pos) const {
        return {pos.x - w, pos.y - h};
    }

    /// Obtains the Position to draw this area so that 'pos' is in the center
    /// horizontally.
    /// @param pos
    /// @return constexpr Position<I>
    [[nodiscard]] constexpr Position<I> horizontal_center(
        Position<I> pos) const {
        return {pos.x - w / 2, pos.y};
    }
    /// Obtains the Position to draw this area so that 'pos' is in the center
    /// vertically.
    /// @param pos
    /// @return constexpr Position<I>
    [[nodiscard]] constexpr Position<I> vertical_center(Position<I> pos) const {
        return {pos.x, pos.y - h / 2};
    }
    /// Obtains the Position to draw this area so that 'pos' is in the center.
    /// @param pos
    /// @return constexpr Position<I>
    [[nodiscard]] constexpr Position<I> center(Position<I> pos) const {
        return {pos.x - w / 2, pos.y - h / 2};
    }

    /// Print in a json-like format.
    /// @param os
    /// @return ostream&
    ostream& print(ostream& os) const {
        os << "{" << w << ", " << h << "}";
        return os;
    }
};

/// Convenience class for expressing color.
struct Color {
    /// Red
    double r;
    /// Green
    double g;
    /// Blue
    double b;
    /// Alpha
    double a;

    /// Default constructor.
    constexpr Color() = default;

    /// Initialize from raw doubles.
    /// @param r
    /// @param g
    /// @param b
    /// @param a
    constexpr Color(double r, double g, double b, double a = 1.0F)
        : r{r}, g{g}, b{b}, a{a} {}

    /// Initialize from hex string (the string you use in HTML).
    /// @param hex
    /// @param a
    constexpr Color(string_view hex, double a = 1.0F)
        : Color{[hex]() {
                    dbg(if (hex.size() != 6) {
                        fatal_error("Not a HEX color code?");
                    });

                    return make_tuple((double)htoi(hex.substr(0, 2)) / 255,
                                      (double)htoi(hex.substr(2, 2)) / 255,
                                      (double)htoi(hex.substr(4, 2)) / 255);
                }(),
                a} {}

    /// Print in a json-like format.
    /// @param os
    /// @return ostream&
    ostream& print(ostream& os) const {
        os << "{ ";
        os << r << ", " << g << ", " << b << ", " << a;
        os << " }";
        return os;
    }

   private:
    /// For clean code.
    /// @param rgb
    /// @param a
    constexpr Color(tuple<double, double, double> rgb, double a)
        : r{get<0>(rgb)}, g{get<1>(rgb)}, b{get<2>(rgb)}, a{a} {}

    /// Convert hex string into unsigned integer.
    /// @param str
    /// @return constexpr auto
    static constexpr unsigned int htoi(string_view str) {
        auto is_hex_digit{[](const char c) {
            return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f');
        }};
        auto hex_to_double{[](const char c) -> unsigned int {
            if ('0' <= c && c <= '9') {
                return c - '0';
            }
            return 10 + (c - 'a');
        }};

        unsigned int base{1};
        unsigned int total{0};
        for (auto itr{str.rbegin()}; itr != str.rend(); itr++) {
            const auto c{*itr};
            // Is hex digit.
            if (!is_hex_digit(c)) {
                fatal_error("Not a hex digit.");
            }

            total += base * hex_to_double(c);
            base *= 16;
        }
        return total;
    };
};
}  // namespace fprd