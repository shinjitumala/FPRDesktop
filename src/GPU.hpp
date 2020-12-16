/// @file GPU.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Threads.hpp>
#include <fprd/Types.hpp>
#include <fprd/Window.hpp>
#include <fprd/draw/Text.hpp>

namespace fprd {
class GPU {
   public:
    using DynamicData = int;
    static constexpr auto probe_interval{1s};
    Position<int> pos;
    const int gpu_count{0};

    GPU(Position<int> pos) : pos{pos} {}

    void update_data(const DynamicData& d) { dbg_out("Update data: " << d); }
    void draw(Window& w, bool new_data) {
        //  dbg_out("Draw: " << new_data);
    }

    static DynamicData get_data() {
        dbg_out("Get data.");
        return 0;
    }
    [[nodiscard]] Window create_window() const {
        Window w{":0.0", pos, {100, 100}};

        TextOnce<VerticalAlign::center>{
            theme::normal, {0, 0}, theme::small_area(50),
            theme::white,  w,      "FUCK"};

        return w;
    }
};

class GPUWindow {
    GPU g;
    Threads<GPU> t;

   public:
    GPUWindow(atomic<bool>& run, Position<int> pos) : g{pos}, t{run, g} {}
};
};  // namespace fprd