/// @file Theme.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Window.hpp>

namespace fprd {
using namespace std;

/// Some preset colors and stuff for some theming.
namespace theme {
const Color black{"000000"};
const Color grey{"7d818d"};
const Color white{"ffffff"};
const Color green{"06ae7a"};
const Color red{"c70700"};
const Color blue{"68c8f2"};
const Color yellow{"fef4ad"};

/// Some preset fonts.
const Font noto_sans{"Noto Sans Mono", CAIRO_FONT_SLANT_NORMAL,
                     CAIRO_FONT_WEIGHT_NORMAL};
const Font noto_sans_bold{"Noto Sans Mono", CAIRO_FONT_SLANT_NORMAL,
                          CAIRO_FONT_WEIGHT_BOLD};

/// Some preset sizes.
constexpr float L1_h{64};
constexpr float L2_h{32};
constexpr float L3_h{16};
constexpr Margin<float> L1_m{8, 8};
constexpr Margin<float> L2_m{4, 4};
constexpr Margin<float> L3_m{2, 2};
constexpr Area<float> L1_area(float w) { return {w, L1_h}; };
constexpr Area<float> L2_area(float w) { return {w, L2_h}; };
constexpr Area<float> L3_area(float w) { return {w, L3_h}; };

template <number I>
constexpr I slow_update(I current, I target) {
    const auto d{target - current};
    if (d == 0) {
        return current;
    }
    return (float)(target - current) / 100 + current;
}

};  // namespace theme
};  // namespace fprd