/// @file AnimatedValue.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Config.hpp>
#include <fprd/Types.hpp>

namespace fprd {
template <number I>
class AnimatedValue {
    I current{0};
    I increment{0};

   public:
    void update(I target_value) {
        increment =
            (static_cast<float>(target_value) - static_cast<float>(current)) /
            fps;
    }

    I draw() {
        current += increment;
        return current;
    }
};
};  // namespace fprd