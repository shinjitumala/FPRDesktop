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
float L1_h{64};
float L2_h{32};
float L3_h{16};
Margin L1_m{4, 4};
Margin L2_m{2, 2};
Margin L3_m{1, 1};
Size L1_area(double w) { return {w, L1_h}; };
Size L2_area(double w) { return {w, L2_h}; };
Size L3_area(double w) { return {w, L3_h}; };

}; // namespace theme
}; // namespace fprd