/// @file CPU.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <dirent.h>
#include <llvm/ADT/SmallString.h>
#include <unistd.h>

#include <chrono>
#include <dbg/Log.hpp>
#include <dbg/Logger.hpp>
#include <filesystem>
#include <fprd/probes/UNIX.hpp>
#include <fprd/util/istream.hpp>
#include <fprd/util/ranges.hpp>
#include <fprd/util/time.hpp>
#include <fprd/util/to_string.hpp>
#include <fstream>
#include <future>
#include <sstream>

namespace fprd {
using namespace ::std;
using namespace ::std::filesystem;

namespace probe::cpu {
/// Utility function for parsing values from UNIX human-readable files.
/// @param is
/// @return auto
auto get_string(istream &is) -> const auto & {
    static string buf;
    skip_to(is, ':');
    getline(is, buf);
    return buf;
}

/// Utility function for parsing values from UNIX human-readable files.
/// @tparam T The format of the value.
/// @param is
/// @return auto
template <class T> auto get_value(istream &is, T &t) -> void {
    skip_to(is, ':');
    skip_to(is, ' ');
    is >> t;
}

/// Information that does not need to be updated after start-up.
struct StaticInfo {
    llvm::SmallString<128> name;
    int threads;
    long mem_total; // KB
};

auto get_info() -> auto {
    StaticInfo info;
    string buf;
    {
        ifstream is{"/proc/cpuinfo"};
        skip_lines(is, 4);
        info.name = get_string(is);
        skip_lines(is, 5);
        get_value(is, info.threads);
    }
    {
        ifstream is{"/proc/meminfo"};
        get_value(is, info.mem_total);
    }
    return info;
}

struct Stat {
    struct Line {
        unsigned long user;
        unsigned long system;
        unsigned long idle;
    };

    Line cpu;
    llvm::SmallVector<Line, 16> threads;
};

auto parse_stat_line(istream &is, Stat::Line &s) -> void {
    skip_to(is, ' ');               // Skip CPU name.
    get<unsigned long>(is, s.user); // 1st value
    skip_to(is, ' ');
    skip_to(is, ' ');                 // Skip 2nd value (nice)
    get<unsigned long>(is, s.system); // 3rd value
    get<unsigned long>(is, s.idle);   // 4th value
    skip_lines(is, 1);
};

auto parse_stat(int thread_count, Stat &data) -> void {
    ifstream is{"/proc/stat"};
    parse_stat_line(is, data.cpu);
    data.threads.resize(thread_count);
    for (auto &t : data.threads) {
        parse_stat_line(is, t);
    }
}

struct CpuInfo {
    llvm::SmallVector<double, 16> freqs;
};

auto parse_cpu_info(int thread_count, CpuInfo &data) -> void {
    ifstream is{"/proc/cpuinfo"};
    skip_lines(is, 7); // Skip to frequency.
    data.freqs.resize(16);
    for (auto &f : data.freqs) {
        get_value(is, f);
        skip_lines(is, 28);
    }
}

struct MemInfo {
    long free; // KB
};

auto parse_mem_info(MemInfo &mi) -> void {
    ifstream is{"/proc/meminfo"};
    skip_lines(is, 2);
    get_value(is, mi.free);
}

auto parse_temp(int &temp) -> void {
    // Ad-hoc way of getting temperatures in FPR's machine in Dec. 2020.
    ifstream is{"/sys/class/thermal/thermal_zone2/temp"};
    get(is, temp);
    temp /= 1000;
}

struct Proc {
    pid_t pid;
    char state;
    unsigned long usage;
    unsigned long mem;
};

using Procs = llvm::SmallVector<Proc, 512>;

auto is_number{[](string_view s) { return llvm::all_of(s, [](auto c) { return '0' <= c && c <= '9'; }); }};

auto parse_procs(Procs &procs) -> void {
    procs.resize(512);

    auto *const dir{opendir("/proc")};
    if (dir == nullptr) {
        fatal_error("Failed to open /proc");
    }

    const dirent *de;
    static llvm::SmallString<64> path_buf;
    int count{0};
    while (de = readdir(dir), de != nullptr) {
        if (!is_number(de->d_name)) {
            continue;
        }
        auto &proc{procs[count]};
        {
            path_buf = "/proc/";
            path_buf.append(de->d_name);
            path_buf.append("/stat");
            ifstream is{path_buf.c_str()};

            get(is, proc.pid); // First data is PID

            /// Skip to usage data.
            skip_to(is, ')');
            get(is, proc.state);
            skip_fields<' '>(is, 11);
            proc.usage = 0;
            for (auto i{0}; i < 4; i++) { // The next four fields are usage times.
                proc.usage += get<unsigned long>(is);
            }
        }
        {
            path_buf = "/proc/";
            path_buf.append(de->d_name);
            path_buf.append("/statm");
            ifstream is{path_buf.c_str()};

            get(is, proc.mem);
            static const auto pagesize{sysconf(_SC_PAGESIZE)};
            proc.mem *= pagesize;
        }

        count++;
        if (512 <= count) {
            cerr << "More than 512 processes!" << endl;
            break;
        }
    }
    closedir(dir);
    procs.resize(count);
}

/// Information that updates frequently.
struct DynamicInfo {
    Stat stat;
    CpuInfo cpu;
    MemInfo mem;
    int temp;
    Procs procs;
};

auto update_data(DynamicInfo &di, int thread_count) -> void {
    parse_stat(thread_count, di.stat);
    parse_cpu_info(thread_count, di.cpu);
    parse_mem_info(di.mem);
    parse_temp(di.temp);
    parse_procs(di.procs);
}
}; // namespace probe::cpu
}; // namespace fprd