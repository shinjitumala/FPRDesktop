/// @file AGraph.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Config.hpp>
#include <fprd/draw/Graph.hpp>

namespace fprd {

/// Line graph
/// @tparam size
/// @tparam Border
/// @tparam FG
/// @tparam BG
template <short size, cairo::source Border = Color, cairo::source FG = Color,
          cairo::source BG = Color>
class AnimatedGraph : public Graph<size, Border, FG, BG> {
    using Base = Graph<size, Border, FG, BG>;
    using Base::draw;

    /// Our class for tracking history.
    class Data {
        using DataArray = typename Base::Data;

        DataArray history{};
        char position{0};

       public:
        /// Obtain the current history, newest first.
        /// @return DataArray
        DataArray get() const {
            DataArray data;
            for (short j{0}, i{position}; j < size; j++, i++, i %= size) {
                data[j] = history[i];
            }
            return data;
        }

        /// Add new data to the history.
        /// @param d
        void add(float d) {
            position--;
            if (position == -1) {
                position = size - 1;
            }
            history[position] = d;
        }
    };

    Data current;

   public:
    AnimatedGraph(Base graph) : Base{graph}, current{} {};
    AnimatedGraph(AnimatedGraph&) = delete;

    void update(float new_value) { current.add(new_value); }

    void draw(Window& w) {
        Base::draw(w, w.frame_counter / fps, current.get());
    }
};
};  // namespace fprd