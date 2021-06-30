/// @file main.cpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

// #include <GPU.hpp>
// #include <System.hpp>
#include <chrono>
#include <csignal>
#include <dbg/Log.hpp>
#include <fprd/probes/CPU.hpp>
#include <fprd/probes/NVML.hpp>
#include <thread>

#include <CPU.hpp>

namespace fprd {
using namespace ::std;
using namespace ::std::chrono;

atomic<CPUWindow *> cpu{nullptr};

/// Signal handler
/// @param signal
/// @return auto
auto stop(int signal) -> void {
    if (cpu != nullptr) {
        cpu.load()->stop();
    }
};

} // namespace fprd

auto main() -> int {
    using namespace ::fprd;
    std::signal(SIGINT, stop);
    std::signal(SIGKILL, stop);

    // Since we do not use C, we can disable this to be faster.
    ios::sync_with_stdio(false);

    CPUWindow w{{0, 0}};
    fprd::cpu = &w;

    return 0;
}