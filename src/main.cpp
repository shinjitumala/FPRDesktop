/// @file main.cpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#include <CPU.hpp>
#include <GPU.hpp>
#include <chrono>
#include <csignal>
#include <dbg/Log.hpp>
#include <fprd/Threads.hpp>
#include <thread>

namespace fprd {
using namespace ::std;
using namespace ::std::chrono;

/// Set to false when exiting.
atomic<bool> run{true};

/// Signal handler
/// @param signal
/// @return auto
auto stop(int signal) { run = false; }

}  // namespace fprd

int main(int argc, char **argv) {
    using namespace ::fprd;
    std::signal(SIGINT, stop);
    std::signal(SIGKILL, stop);

    GPUWindow gpus{run, {0, 0}};
    CPUWindow cpu{run, CPUWindow::area.top_right({1920, 0})};

    return 0;
}