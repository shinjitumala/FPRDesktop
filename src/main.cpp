/// @file main.cpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#include <chrono>
#include <csignal>
#include <dbg/Log.hpp>
#include <fprd/widgets/CPU.hpp>
#include <fprd/widgets/Nvidia.hpp>
#include <thread>

namespace fprd {
using namespace ::std;
using namespace ::std::chrono;

/// Set to false when exiting.
atomic<bool> run{true};
/// Draw interval.
constexpr auto interval{16ms};

constexpr auto draw_loop{[&run = run](auto f) {
    while (run) {
        const auto tp{now()};
        f();
        this_thread::sleep_until(tp + interval);
    }
}};

/// Signal handler
/// @param signal
/// @return auto
auto stop(int signal) { run = false; }

/// The window with GPU stats
class GPUWindow {
    nvml::NVML nvml{};
    vector<nvml::Device> devs;

    FPRWindow w;

    vector<widget::Nvidia> gpus;

    /// Drawing thread.
    thread t;

   public:
    GPUWindow()
        : nvml{},
          devs{nvml.get_devices(run)},
          w{":0.0", {0, 0}, widget::Nvidia::size.scale({devs.size(), 1})},
          gpus{[&]() {
              vector<widget::Nvidia> gpus;
              for (auto i{0U}; i < devs.size(); i++) {
                  gpus.emplace_back(widget::Nvidia{
                      w, devs[i], {i * widget::Nvidia::size.w, 0}});
              }
              return gpus;
          }()},
          t{draw_loop, [&]() { draw(); }} {}

    GPUWindow(const GPUWindow &) = delete;
    ~GPUWindow() { t.join(); }

   private:
    void draw() {
        for_each(gpus.begin(), gpus.end(),
                 [this](auto &widget) { widget.draw(w); });
        w.flush();
    }
};

/// The window with CPU stats
struct CPUWindow {
    query::CPU q;
    FPRWindow win;
    widget::CPU widget;

    /// Drawing thread.
    thread t;

    CPUWindow()
        : q{run},
          win{":0.0", widget::CPU::size.top_right({1920, 0}),
              widget::CPU::size},
          widget{win, q, {0, 0}},
          t{draw_loop, [&]() { draw(); }} {}
    ~CPUWindow() { t.join(); }

    void draw() {
        widget.draw(win);
        win.flush();
    }
};
}  // namespace fprd

int main(int argc, char **argv) {
    using namespace ::fprd;
    std::signal(SIGINT, stop);
    std::signal(SIGKILL, stop);

    GPUWindow gpus{};
    CPUWindow cpu{};

    return 0;
}