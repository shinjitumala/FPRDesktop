/// @file Graph.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <fprd/Config.hpp>
#include <fprd/Types.hpp>
#include <fprd/Window.hpp>
#include <fprd/util/ranges.hpp>
#include <ranges>

namespace fprd {
using namespace std;

/// Our class for tracking history.
template <short size>
struct Data {
    using DataType = float;
    using DataArray = array<DataType, size>;

   private:
    DataArray history{};
    char position{0};

   public:
    /// Obtain the current history, newest first.
    /// @return DataArray
    DataArray get() const {
        DataArray data;
        for (char j{0}, i{position}; j < size; j++, i++, i %= size) {
            data[j] = history[i];
        }
        return data;
    }

    /// Add new data to the history.
    /// @param d
    void add(DataType d) {
        position--;
        if (position == -1) {
            position = size - 1;
        }
        history[position] = d;
    }
};

/// Line graph
/// @tparam size
/// @tparam Border
/// @tparam FG
/// @tparam BG
template <short size, cairo::source Border = Color, cairo::source FG = Color,
          cairo::source BG = Color>
struct Graph {
   public:
    /// The data that is drawn.
    using Data = array<float, size>;

    Position<float> pos;
    Area<float> area;

    Border b;
    float border_width;
    FG fg;
    BG bg;

    /// Draw
    /// @param w
    /// @param offset The offset in fractions. 0 means the first data is
    /// completely hidden, and 1 means it is visible right at the edge.
    /// @param data
    void draw(Window &w, float offset_factor, Data data) {
        /// Fill background
        w.rectangle(pos, area);
        w.fill();

        /// Fill graph.
        const auto interval{area.w / (size - 2)};
        const auto offset{offset_factor * interval};
        w.set_source(fg);
        {
            const auto dif{data[1] - data[0]};
            w.move_to(
                pos.offset({0, area.h - (data[0] + dif * offset / interval)}));
        }
        for (auto [i, d] :
             data | enumerate | ::std::ranges::views::take(size - 1)) {
            w.line_to(
                pos.offset({offset + i * interval, area.h * (100 - d) / 100}));
        }
        {
            const auto dif{data[size - 1] - data[size - 2]};
            w.line_to(pos.offset(
                {area.w, area.h - (data[size - 2] + dif * offset / interval)}));
        }
        w.line_to(pos.offset({area.w, area.h}));
        w.line_to(pos.offset({0, area.h}));
        w.fill();

        /// Border
        w.set_source(b);
        w.set_line_width(border_width);
        w.rectangle(pos, area);
        w.stroke();
    }
};

};  // namespace fprd