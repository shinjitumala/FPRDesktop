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

/// Animated line graph.
/// @tparam size The size of the history.
/// @tparam Border
/// @tparam FG
/// @tparam BG
template <short size, cairo::source Border = Color, cairo::source FG = Color, cairo::source BG = Color>
class AnimatedGraph : public Graph<size, Border, FG, BG> {
    using Base = Graph<size, Border, FG, BG>;
    using Base::draw;

    /// Our class for tracking history.
    class Data {
        using DataArray = typename Base::Data;

        /// Store the data here.
        DataArray history{};

        /// The current position.
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

    /// Stored history.
    Data current;

  public:
    /// Initialize from a Graph.
    /// @param graph
    AnimatedGraph(Base graph) : Base{graph}, current{} {};
    /// Copying is not allowed.
    AnimatedGraph(const AnimatedGraph &) = delete;
    /// Moving is allowed, however.
    AnimatedGraph(AnimatedGraph &&) noexcept = default;

    /// Update the target percentage.
    /// Currently hard-coded such that this function must be called every second
    /// or it breaks.
    /// TODO: Make the update interval adjustable via template parameters.
    /// @param new_value
    void update(float new_value) {
        if (100 < new_value) {
            dbg::err << "Out of bounds: 100 < " << new_value << endl;
            new_value = 100;
        }
        if (new_value < 0) {
            dbg::err << "Out of bounds: " << new_value << " < 0 " << endl;
            new_value = 0;
        }

        current.add(new_value);
    }

    /// Call this every frame.
    /// @param w
    void draw(Window &w) { Base::draw(w, (float)w.frame_counter / fps, current.get()); }
};
}; // namespace fprd