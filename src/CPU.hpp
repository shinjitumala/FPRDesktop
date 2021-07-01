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
#include <fprd/probes/CPU.hpp>
#include <fprd/probes/NVML.hpp>
#include <fprd/util/ranges.hpp>
#include <thread>

namespace fprd {
using namespace std;

class CPUWindow {
    /// Used to store intermediate data required to gather the presented data.
    /// In this case, we have to sample the time spent in system/user ourselves.
    struct State {
        probe::cpu::Stat stat;
    };

  public:
    static constexpr int width{64};
    static constexpr int height{36};

  private:
    using OurWindow = Window<width, height>;

    OurWindow w;

    const probe::cpu::StaticInfo si;
    probe::cpu::DynamicInfo di;

    OurWindow::Lines buf;

    atomic<bool> run;
    thread t;

    State state;

  public:
    CPUWindow(Position<float> pos) : w{"", pos}, si{probe::cpu::get_info()}, run{true}, t{[this] { runner(); }} {}
    ~CPUWindow() { t.join(); }

    auto stop() -> void { run = false; }

  private:
    auto runner() -> void {
        w.update(buf); // Update once to make the window background appar immediately.

        int thread_count{stoi(si.threads.str().data())};

        {
            // CPU name at the header.
            const auto left_margin{(width - si.name.size()) / 2};
            print_to(buf[0], "%*c%s", left_margin, ' ', si.name.data());
        }

        // The initial probing.
        probe::cpu::update_data(di, thread_count);
        update_state();

        while (run) {
            auto tp{now()};

            w.update(buf);
            probe::cpu::update_data(di, thread_count);
            update_buf();
            update_state();

            this_thread::sleep_until(tp + 1s);
        }
    }

    auto update_buf() -> void {
        {
            const auto &last_cpu{state.stat.cpu};
            const auto &cur_cpu{di.stat.cpu};
            const auto cpu_idle{cur_cpu.idle - last_cpu.idle};
            const auto cpu_active{(cur_cpu.user - last_cpu.user) + (cur_cpu.user - last_cpu.user)};

            const auto util{(long double)cpu_active / (cpu_active + cpu_idle) * 100};
            print_to(buf[1], "Utilization: %.2Lf%%", util);
        }
    }

    auto update_state() -> void { state.stat = di.stat; }
};
}; // namespace fprd