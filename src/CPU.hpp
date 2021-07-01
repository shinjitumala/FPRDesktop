/// @file CPU.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Theme.hpp>
#include <fprd/Window.hpp>
#include <fprd/element/Bar.hpp>
#include <fprd/probes/CPU.hpp>
#include <fprd/probes/NVML.hpp>
#include <fprd/util/ranges.hpp>
#include <thread>

namespace fprd {
using namespace std;

namespace cpu {
/// Storage type for data that requires storing the previous value to present.
struct PrevInfo {
    probe::cpu::Stat stat;
};

/// Storage required for drawing.
struct StateDraw {
    element::Bar<element::bar_standard> util;
    element::Bar<element::bar_standard> mem;
};

/// Data used directly for presenting.
struct StateInfo {
    long double util;     // Ratio [0, 1]
    long double mem;      // Ratio [0, 1]
    long double mem_used; // GB
};

class Widget {
  public:
    static constexpr int width{64};
    static constexpr int height{36};

    static constexpr int left_end{21};

  private:
    using Window = fprd::Window<width, height>;

    Window w;

    const probe::cpu::StaticInfo si;
    probe::cpu::DynamicInfo di;

    Window::Lines buf;

    PrevInfo prev;
    StateInfo state_i;
    StateDraw state_d;

    atomic<bool> run;
    thread t;

  public:
    Widget(Position<float> pos)
        : w{"", pos}, si{probe::cpu::get_info()}, state_d{init_state_draw(buf)}, run{true}, t{[this] {
              runner();
          }} {}
    ~Widget() { t.join(); }

    auto stop() -> void { run = false; }

  private:
    static auto init_state_draw(Window::Lines &buf) -> StateDraw {
        StateDraw sd{
            .util = {buf[1][left_end - 1], buf[1][width]},
            .mem = {buf[2][left_end - 1], buf[2][width]},
        };
        return sd;
    };

    static auto init_buf(decltype(buf) &buf, decltype(si) &si) {
        {
            // CPU name at the header.
            const auto left_margin{(width - si.name.size()) / 2};
            print_to(buf[0], "%*c%s", left_margin, ' ', si.name.data());
        }
        print_to_st(buf[1], "Util");
        print_to_st(buf[2], "Mem");
        print_to_st(buf[2], left_end - 1 - 7, "/%3.0LfGB", si.mem_total * 1e-6L);
    }

    auto runner() -> void {
        w.update(buf); // Update once to make the window background appar immediately.

        int thread_count{si.threads};

        init_buf(buf, si);

        // The initial probing.
        probe::cpu::update_data(di, thread_count);
        update_prev(di, prev);

        while (run) {
            auto tp{now()};

            w.update(buf);
            probe::cpu::update_data(di, thread_count);
            update_state_info(si, di, prev, state_i);
            update_buf(state_i, state_d, buf);
            update_prev(di, prev);

            this_thread::sleep_until(tp + 1s);
        }
    }

    /// Update StateDraw and buf with the current StateInfo.
    static auto update_buf(const StateInfo &si, StateDraw &sd, decltype(buf) &buf) -> void {
        // Utilization
        print_to_st(buf[1], 4, "%*c%3.2Lf%%", left_end - 4 - 7, ' ', si.util * 100);
        sd.util.update(si.util);

        // Memory
        sd.mem.update(si.mem);
        print_to_st(buf[2], left_end - 1 - 7 - 6, "%3.2Lf", si.mem_used);
        buf[2][left_end - 1 - 7] = '/';
    }

    /// Process StaticInfo, DynamicInfo,and  PrevInfo to update the StateInfo.
    static auto update_state_info(decltype(si) &si, const decltype(di) &di, const decltype(prev) &prev,
                                  decltype(state_i) &state) -> void {
        state.util = [&] {
            const auto &last_cpu{prev.stat.cpu};
            const auto &cur_cpu{di.stat.cpu};
            const auto cpu_idle{cur_cpu.idle - last_cpu.idle};
            const auto cpu_active{(cur_cpu.user - last_cpu.user) + (cur_cpu.user - last_cpu.user)};

            return (long double)cpu_active / (cpu_active + cpu_idle);
        }();
        tie(state.mem, state.mem_used) = [&] {
            const auto used_kb{si.mem_total - di.mem.free}; // KB
            const auto used_gb{used_kb * 1e-6L};            // GB
            const auto used_ratio{(long double)used_kb / si.mem_total};
            return make_pair(used_ratio, used_gb);
        }();
    }

    /// Update prev with the current DynamicInfo.
    static auto update_prev(const decltype(di) &di, decltype(prev) &prev) -> void { prev.stat = di.stat; }
};
}; // namespace cpu
}; // namespace fprd