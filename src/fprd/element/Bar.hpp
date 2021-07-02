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
#include <fprd/Window.hpp>
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
template <BarConfig cfg, class Window> class Bar {
    const size_t line;
    const size_t start;
    const size_t end;

  public:
    Bar(typename Window::Lines &lines, size_t line, size_t start, size_t end)
        : line{line}, start{start}, end{end} {
        if (end - start < 4) {
            fatal_error("A `Bar`'s size cannot be less than 4.");
        }

        auto &l{lines[line]};
        l[start] = cfg.lb;
        l[end - 1] = cfg.rb;
    }

    /// @param ratio How much the bar is filled. Minimum 0, maximum 1.
    auto update(typename Window::Lines &lines, long double ratio) -> void {
        // Minus the brackets
        const auto inner_size{end - start - 2};
        const auto filled{static_cast<long>(inner_size * ratio)};
        auto &l{lines[line]};
        fill(&l[start + 1], &l[start + 1 + filled], cfg.fill);
        l[start + 1 + filled] = cfg.edge;
        fill(&l[start + filled + 2], &l[end - 1], cfg.empty);
    };
};
}; // namespace element
} // namespace fprd