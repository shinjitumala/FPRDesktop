/// @file Bar.hpp
/// @author FPR (funny.pig.run__AT_MARK_gmail.com)
/// @version 0.0
/// @date 2021-07-01
///
/// @copyright Copyright (c) 2021
///

#pragma once

#include <cmath>
#include <dbg/Log.hpp>
#include <fprd/util/format.hpp>
#include <span>

namespace fprd {
using namespace std;

/// Simple classes/functions to generate strings that represents bars/graphs easily.
namespace element {
struct BarConfig {
    /// Left bracket
    char lb;
    /// Right bracket
    char rb;
    /// Filled
    char fill;
    /// Empty
    char empty;
    /// Edge (of the filled bar)
    char edge;
};

constexpr BarConfig bar_standard{
    '[', ']', '=', '-', '>',
};

/// A horizontal bar.
template <BarConfig cfg> class Bar {
    span<char> view;

  public:
    Bar(char &start, char &end) : view{&start, &end} {
        if (view.size() < 4) {
            fatal_error("A `Bar`'s size cannot be less than 4.");
        }

        view.front() = cfg.lb;
        view.back() = cfg.rb;
    }

    /// @param ratio How much the bar is filled. Minimum 0, maximum 1.
    auto update(long double ratio) -> void {
        // Minus the brackets
        const auto inner_size{view.size() - 2};
        const auto filled{static_cast<long>(inner_size * ratio)};
        fill(view.begin() + 1, view.begin() + (1 + filled), cfg.fill);
        view[1 + filled] = cfg.edge;
        fill(view.begin() + (1 + filled + 1), view.end() - 1, cfg.empty);
    };
};
}; // namespace element
} // namespace fprd