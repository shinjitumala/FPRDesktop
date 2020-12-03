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
// #include <fprd/widgets/CPU.hpp>
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

    GPUWindow()
        : nvml{},
          devs{nvml.get_devices(run)},
          win{":0.0", {0, 0}, widget::Nvidia::size.scale({devs.size(), 1})},
          gpus{[&]() {
              vector<widget::Nvidia> gpus;
              for (auto i{0U}; i < devs.size(); i++) {
                  gpus.emplace_back(widget::Nvidia{
                      win, devs[i], {i * widget::Nvidia::size.w, 0}});
              }
              return gpus;
          }()} {}

    GPUWindow(const GPUWindow &) = delete;

    void draw() {
        for_each(gpus.begin(), gpus.end(),
                 [this](auto &widget) { widget.draw(win); });
        win.flush();
    }
};

// struct CPUWindow {
//     query::CPU q;
//     FPRWindow win;
//     widget::CPU widget;

//     CPUWindow()
//         : q{},
//           win{":0.0", widget::CPU::size.top_right({1920, 0}),
//               widget::CPU::size},
//           widget{win, q, {0, 0}} {}

//     void start_update() { q.start_update(); }

//     void draw() {
//         widget.draw(win);
//         win.flush();
//     }
// };

int main(int argc, char **argv) {
    std::signal(SIGINT, stop);
    std::signal(SIGKILL, stop);

    GPUWindow gpus{};
    // CPUWindow cpu{};

    int count{};
    for (; run; count++, count %= 3600) {
        using namespace chrono;
        const auto last{time_point<high_resolution_clock>::clock::now()};

        // if (count % 60 == 0) {
        //     cpu.start_update();
        // }

        gpus.draw();
        // cpu.draw();

        // dbg(cout << duration_cast<milliseconds>(
        //                 time_point<high_resolution_clock>::clock::now() -
        //                 last) .count()
        //          << "ms" << endl;);

        this_thread::sleep_until(last + 16ms);
    }

    return 0;
}