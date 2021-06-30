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
constexpr Color white{"ffffff"};

/// Some preset fonts.
const cairo::Font normal{"Noto Sans Mono", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL};
}; // namespace theme
}; // namespace fprd