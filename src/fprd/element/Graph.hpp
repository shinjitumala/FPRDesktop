/// @file Graph.hpp
/// @author FPR (funny.pig.run__AT_MARK_gmail.com)
/// @version 0.0
/// @date 2021-07-01
///
/// @copyright Copyright (c) 2021
///

#pragma once

#include "fprd/util/ranges.hpp"
#include <cmath>
#include <dbg/Log.hpp>
#include <fprd/Types.hpp>
#include <fprd/util/format.hpp>
#include <span>
#include <vector>

namespace fprd {
using namespace std;
namespace element {

struct GraphConfig {
    /// Top & bottom
    char tb;
    /// Left & right
    char lr;
    /// Filled
    char f;
    /// Empty
    char e;
};

constexpr GraphConfig graph_standard{'-', '|', '=', ' '};

template <GraphConfig cfg, class Window> class VGraph {
    using Lines = typename Window::Lines;

    Position<int> tl;
    Position<int> br;
    vector<char> history;
    size_t idx;

  public:
    /// @param lines
    /// @param tl Top left
    /// @param br Bottom right
    VGraph(Lines &lines, Position<int> tl, Position<int> br) : tl{tl}, br{br}, history(br.x - tl.x - 2), idx{0} {

        auto &fl{lines[tl.y]};
        auto &ll{lines[br.y - 1]};

        fill(&fl[tl.x], &fl[br.x], cfg.tb);
        fill(&ll[tl.x], &ll[br.x], cfg.tb);
        for_each(&lines[tl.y + 1], &lines[br.y - 1], [&](auto &l) {
            l[tl.x] = cfg.lr;
            l[br.x - 1] = cfg.lr;
        });
    }

    auto update(Lines &lines, long double ratio) -> void {
        if (!isfinite(ratio)) {
            ratio = 1;
        }
        const auto inner_w{br.x - tl.x - 2};
        const auto inner_h{br.y - tl.y - 2};
        history[idx] = ratio * inner_h;
        auto lidx{0};
        for_each(&lines[tl.y + 1], &lines[br.y - 1], [&](auto &l) {
            const auto h{inner_h - lidx};
            for (auto i{tl.x + 1}; i < br.x - 1; i++) {
                const auto hidx{i - (tl.x + 1)};
                if (h <= history[(inner_w + idx - hidx) % inner_w]) {
                    l[i] = cfg.f;
                } else {
                    l[i] = cfg.e;
                }
            }
            lidx++;
        });
        idx++;
        idx %= inner_w;
    }
};

template <GraphConfig cfg, class Window> class HGraph {
    using Lines = typename Window::Lines;

    Position<int> tl;
    Position<int> br;
    vector<char> history;
    size_t idx;

  public:
    /// @param lines
    /// @param tl Top left
    /// @param br Bottom right
    HGraph(Lines &lines, Position<int> tl, Position<int> br) : tl{tl}, br{br}, history(br.y - tl.y), idx{0} {}

    auto update(Lines &lines, long double ratio) -> void {
        if (!isfinite(ratio)) {
            ratio = 1;
        }
        const auto inner_h{br.y - tl.y};
        const auto inner_w{br.x - tl.x};
        history[idx] = ratio * inner_w;
        auto lidx{0};
        for_each(&lines[tl.y], &lines[br.y], [&](auto &l) {
            const auto filled{history[(inner_h + idx - lidx) % inner_h]};
            for (auto i{tl.x}; i < tl.x + filled; i++) {
                l[i] = cfg.f;
            }
            for (auto i{tl.x + filled}; i < br.x; i++) {
                l[i] = cfg.e;
            }
            lidx++;
        });
        idx++;
        idx %= inner_h;
    }
};
} // namespace element
}; // namespace fprd
