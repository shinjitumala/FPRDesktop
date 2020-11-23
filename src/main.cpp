/// @file main.cpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#include <algorithm>
#include <chrono>
#include <csignal>
#include <dbg/Log.hpp>
#include <fprd/Config.hpp>
#include <fprd/NVML.hpp>
#include <fprd/Pattern.hpp>
#include <fprd/System.hpp>
#include <fprd/Theme.hpp>
#include <fprd/Window.hpp>
#include <fprd/parts/ArcBar.hpp>
#include <fprd/parts/Text.hpp>
#include <fprd/query/CPU.hpp>
#include <fprd/util/to_string.hpp>
#include <fprd/widgets/Nvidia.hpp>
#include <fstream>
#include <ranges>
#include <span>
#include <thread>

using namespace std;
using namespace fprd;

atomic<bool> run{true};

auto stop(int signal) { run = false; }

struct GPUWindow {
    nvml::NVML nvml{};
    vector<nvml::Device> devs;

    FPRWindow win;

    vector<widget::Nvidia> gpus;

    GPUWindow(const X11 &x11)
        : nvml{},
          devs{nvml.get_devices()},
          win{x11,
              {0, 0},
              {static_cast<unsigned int>(devs.size() * widget::Nvidia::w),
               static_cast<unsigned int>(widget::Nvidia::h)}},
          gpus{[&]() {
              vector<widget::Nvidia> gpus;
              for (auto i{0U}; i < devs.size(); i++) {
                  gpus.emplace_back(
                      widget::Nvidia{win, devs[i], {i * widget::Nvidia::w, 0}});
              }
              return gpus;
          }()} {}

    GPUWindow(const GPUWindow &) = delete;

    void start_update() {
        for_each(gpus.begin(), gpus.end(), [](auto &w) { w.start_update(); });
    }

    void draw() {
        for_each(gpus.begin(), gpus.end(),
                 [this](auto &widget) { widget.draw(win); });
        win.flush();
    }
};

struct DateTimeWindow {};

int main(int argc, char **argv) {
    std::signal(SIGINT, stop);
    std::signal(SIGKILL, stop);

    const X11 x11{":0.0"};

    query::CPU cpu{};

    GPUWindow gpus{x11};

    int count{};
    for (; run; count++, count %= 3600) {
        using namespace chrono;
        const auto last{time_point<high_resolution_clock>::clock::now()};

        if (count % 60 == 0) {
            gpus.start_update();
            cpu.start_update();
        }

        gpus.draw();
        cpu.check_update();

        dbg(cout << duration_cast<milliseconds>(
                        time_point<high_resolution_clock>::clock::now() - last)
                        .count()
                 << "ms" << endl;);

        this_thread::sleep_until(last + 16ms);
    }

    return 0;
}