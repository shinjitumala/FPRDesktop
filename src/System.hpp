/// @file System.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2021
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <chrono>
#include <fprd/Theme.hpp>
#include <fprd/Threads.hpp>
#include <fprd/Window.hpp>
#include <fprd/draw/Text.hpp>

namespace fprd {
using namespace std;
class System {
   public:
    struct DynamicData {
        time_t t;
    };
    static inline const auto probe_interval{1s};
    static constexpr Area<float> area{256, 256};

    Position<float> pos;

   private:
    string time;
    TextCleared<VerticalAlign::center> t_time;

   public:
    System(Position<float> pos)
        : pos{pos},
          t_time{{&theme::normal, {0, 0}, theme::medium_area(area.w)},
                 theme::white, theme::black} {}

    void update_data(DynamicData d) { time = ctime(&d.t); }

    void draw(Window &w, bool updated) {
        if (updated) {
            t_time.draw(w, time);
        }
    }

    DynamicData get_data() {
        using namespace chrono;
        DynamicData d;
        d.t = system_clock::to_time_t(system_clock::now());
        return d;
    }

    Window create_window() {
        Window w{":0.0", pos, area};
        return w;
    }
};

class SystemWindow {
    System p;
    Threads<System> t;

   public:
    static constexpr Area<float> area{System::area};

    SystemWindow(atomic<bool> &run, Position<int> pos) : p{pos}, t{run, p} {}
};
}  // namespace fprd