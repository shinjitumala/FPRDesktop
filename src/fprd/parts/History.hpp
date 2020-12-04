/// @file History.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <array>
#include <cstddef>
#include <fprd/Utils.hpp>
#include <fprd/Window.hpp>
#include <fprd/util/ranges.hpp>

namespace fprd {
using namespace std;

/// Draw data with history.
template <number I, Source Border = Color, Source FG = Color, Source BG = Color>
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
    void update(FPRWindow& w, I new_data) {
        history.add(new_data);
        w.set_source(bg);
        w.rectangle(pos, area);
        w.fill();

        w.set_source(b);
        w.set_line_width(border_width);
        w.rectangle(pos, area);
        w.stroke();

        const auto interval{area.w / (size - 1)};
        w.set_source(fg);
        w.move_to(pos.offset({0, area.h}));
        const auto data{history.get()};
        for (auto [i, d] : data | enumerate) {
            w.line_to(pos.offset({i * interval, area.h * (100 - d) / 100}));
        }
        w.line_to(pos.offset({area.w, area.h}));
        w.line_to(pos.offset({0, area.h}));
        w.fill();
    }
};

};  // namespace fprd