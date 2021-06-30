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

  public:
    CPUWindow(Position<float> pos) : w{"", pos}, si{probe::cpu::get_info()}, run{true}, t{[this] { runner(); }} {}
    ~CPUWindow() { t.join(); }

    auto stop() -> void { run = false; }

  private:
    auto runner() -> void {
        w.update(buf);
        int thread_count{stoi(si.threads.str().data())};

        buf[0] = [&name = si.name] {
            const auto size{name.size()};
            string str((width - size) / 2, ' ');
            str.append(name.begin(), name.end());
            return str;
        }();

        probe::cpu::update_data(di, thread_count);
        while (run) {
            auto tp{now()};
            w.update(buf);
            probe::cpu::update_data(di, thread_count);

            this_thread::sleep_until(tp + 1s);
        }
    }
};
}; // namespace fprd