/// @file Theme.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Config.hpp>
#include <fprd/wrapper/Cairo.hpp>

namespace fprd {
using namespace std;

/// Some preset colors and stuff for some theming.
namespace theme {
constexpr Color black{"000000"};
constexpr Color grey{"7d818d"};
constexpr Color white{"ffffff"};
constexpr Color green{"06ae7a"};
constexpr Color red{"c70700"};
constexpr Color blue{"68c8f2"};
constexpr Color yellow{"fef4ad"};

/// Some preset fonts.
const cairo::Font normal{"Noto Sans Mono", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_NORMAL};
const cairo::Font bold{"Noto Sans Mono", CAIRO_FONT_SLANT_NORMAL,
                       CAIRO_FONT_WEIGHT_BOLD};

/// Some preset sizes.
constexpr auto large_h{28};
constexpr auto medium_h{20};
constexpr auto small_h{16};
constexpr Area<float> large_area(float w) { return {w, large_h}; };
constexpr Area<float> medium_area(float w) { return {w, medium_h}; };
constexpr Area<float> small_area(float w) { return {w, small_h}; };

/// Compute interval based on Config
template <number I>
auto get_interval(I target, I current) {
    return (target - current) / fps;
}
};  // namespace theme
};  // namespace fprd