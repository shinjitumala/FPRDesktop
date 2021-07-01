/// @file Graph.hpp
/// @author FPR (funny.pig.run__AT_MARK_gmail.com)
/// @version 0.0
/// @date 2021-07-01
///
/// @copyright Copyright (c) 2021
///

#pragma once

#include "fprd/util/ranges.hpp"
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

constexpr GraphConfig graph_standard{'-', '|', ':', ' '};

template <GraphConfig cfg> class Graph {
    vector<span<char>> line_refs;
    vector<char> history;
    size_t idx;

  public:
    /// @tparam W
    /// @tparam H
    /// @param lines
    /// @param lt Top left
    /// @param rb Bottom right
    template <class Lines>
    Graph(Lines &lines, Position<int> lt, Position<int> rb)
        : line_refs{[&] {
              vector<span<char>> ret;
              const auto hs{lt.x};
              const auto he{rb.x};
              const auto vs{lt.y};
              const auto ve{rb.y};
              for (auto y{vs}; y <= ve; y++) {
                  auto &l{lines[y]};
                  ret.push_back({l.begin() + hs, l.begin() + he});
              }
              return ret;
          }()},
          history(line_refs[0].size() - 2), idx{0} {

        auto firstline{line_refs.front()};
        auto lastline{line_refs.back()};

        fill(firstline.begin(), firstline.end(), cfg.tb);
        fill(lastline.begin(), lastline.end(), cfg.tb);
        for_each(line_refs.begin() + 1, line_refs.end() - 1, [](auto &l) {
            l.front() = cfg.lr;
            l.back() = cfg.lr;
        });
    }

    auto update(long double ratio) -> void {
        const auto width{history.size()};
        const auto height{line_refs.size() - 2};
        history[idx] = ratio * height;
        auto lidx{0};
        for_each(line_refs.begin() + 1, line_refs.end() - 1, [&](auto &l) {
            for (auto i{idx}; i != (idx - 1) % width; i++, i %= width) {
                const auto pidx{(width + i - idx) % width + 1};
                if ((height - lidx) <= history[i]) {
                    l[pidx] = cfg.f;
                } else {
                    l[pidx] = cfg.e;
                }
            }
            lidx++;
        });
        idx--;
        idx += width;
        idx %= width;
    }
};
} // namespace element
}; // namespace fprd