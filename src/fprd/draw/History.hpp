/// @file History.hpp
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

/// Draw data with history.
template <number I, cairo::source Border = Color, cairo::source FG = Color,
          cairo::source BG = Color>
struct History {
    /// The size of our history.
    inline static constexpr auto size{40};

   private:
    /// Our class for tracking history.
    struct Data {
        using DataType = I;
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

    Data history;
    /// offset used to animate.
    float offset;

   public:
    Position<float> pos;
    Area<float> area;

    Border b;
    float border_width;
    FG fg;
    BG bg;

    /// Draw history with new data.
    /// @param w
    /// @param new_data
    void update(I new_data) { history.add(new_data); }

    /// Draw
    void draw(Window &w) {
        w.rectangle(pos, area);
        w.fill();

        w.set_source(b);
        w.set_line_width(border_width);
        w.rectangle(pos, area);
        w.stroke();

        const auto interval{area.w / (size - 2)};
        const auto offset_increment{interval / fps};
        offset += offset_increment;
        offset = fmodf(offset, interval);

        w.set_source(fg);
        /// The current data remains hidden.

        const auto data{history.get()};
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
            w.move_to(pos.offset(
                {area.w, area.h - (data[size - 2] + dif * offset / interval)}));
        }
        w.line_to(pos.offset({area.w, area.h}));
        w.line_to(pos.offset({0, area.h}));
        w.fill();
    }
};

};  // namespace fprd