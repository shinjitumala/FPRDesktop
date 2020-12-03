/// @file Util.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <concepts>
#include <ostream>
#include <sstream>

namespace fprd {
using namespace ::std;

template <class I>
concept number = integral<I> || floating_point<I>;

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

template <number I>
struct Margin {
    I l;
    I r;
    I t;
    I b;

    constexpr Margin() = default;
    constexpr Margin(I l, I r, I t, I b) : l{l}, r{r}, t{t}, b{b} {}
    constexpr Margin(I x, I y) : l{x}, r{y}, t{y}, b{y} {}

    constexpr Margin operator+(Margin rhs) const {
        return {l + rhs.l, r + rhs.r, t + rhs.t, b + rhs.b};
    }
    template <number S>
    requires convertible_to<S, I> constexpr Margin operator*(S scale) const {
        I s{static_cast<I>(scale)};
        return {l * s, r * s, t * s, b * s};
    };
    template <number S>
    requires convertible_to<I, S> constexpr operator Margin<S>() const {
        if constexpr (is_floating_point_v<I> && is_integral_v<S>) {
            return {round<S>(l), round<S>(r), round<S>(t), round<S>(b)};
        }
        return {static_cast<S>(l), static_cast<S>(r), static_cast<S>(t),
                static_cast<S>(b)};
    }
};

template <number I>
struct Area;

template <number I>
struct Position {
    I x;
    I y;

    constexpr Position() = default;
    constexpr Position(I x, I y) : x{x}, y{y} {}

    /// IMPORTANT: Always pad first before applying area adjustments.
    /// @param m
    /// @return constexpr Position
    [[nodiscard]] constexpr Position pad(Margin<I> m) const {
        return {x + m.l, y + m.t};
    }

    constexpr Position operator+(Position rhs) const {
        return {x + rhs.x, y + rhs.y};
    }

    constexpr Position stack_right(Area<I> a) { return {x + a.w, y}; }
    constexpr Position stack_bottom(Area<I> a) { return {x, y + a.h}; }

    /// Becase we can't type deduct with 'operator+'?
    /// @param rhs
    /// @return constexpr Position
    constexpr Position offset(Position rhs) const { return *this + rhs; }

    template <number S>
    requires convertible_to<I, S> constexpr operator Position<S>() const {
        if constexpr (is_floating_point_v<I> && is_integral_v<S>) {
            return {round<S>(x), round<S>(y)};
        }
        return {static_cast<S>(x), static_cast<S>(y)};
    }

    ostream& print(ostream& os) const {
        os << "{" << x << ", " << y << "}";
        return os;
    }
};

template <number I>
struct Area {
    I w;
    I h;

    constexpr Area() = default;
    constexpr Area(I w, I h) : w{w}, h{h} {}

    [[nodiscard]] constexpr Area pad(Margin<I> m) const {
        return {w - m.l - m.r, h - m.t - m.b};
    };
    [[nodiscard]] constexpr Area scale(pair<float, float> scale) const {
        return {w * scale.first, h * scale.second};
    }
    template <number S>
    requires convertible_to<I, S> constexpr operator Area<S>() const {
        if constexpr (is_floating_point_v<I> && is_integral_v<S>) {
            return {round<S>(w), round<S>(h)};
        }
        return {static_cast<S>(w), static_cast<S>(h)};
    }

    [[nodiscard]] constexpr Position<I> bottom_left(Position<I> pos) const {
        return {pos.x, pos.y - h};
    }
    [[nodiscard]] constexpr Position<I> top_right(Position<I> pos) const {
        return {pos.x - w, pos.y};
    }
    [[nodiscard]] constexpr Position<I> bottom_right(Position<I> pos) const {
        return {pos.x - w, pos.y - h};
    }
    [[nodiscard]] constexpr Position<I> horizontal_center(
        Position<I> pos) const {
        return {pos.x - w / 2, pos.y};
    }
    [[nodiscard]] constexpr Position<I> vertical_center(Position<I> pos) const {
        return {pos.x, pos.y - h / 2};
    }
    [[nodiscard]] constexpr Position<I> center(Position<I> pos) const {
        return {pos.x - w / 2, pos.y - h / 2};
    }

    ostream& print(ostream& os) const {
        os << "{" << w << ", " << h << "}";
        return os;
    }
};
}  // namespace fprd