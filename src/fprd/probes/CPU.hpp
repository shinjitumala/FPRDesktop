/// @file CPU.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <llvm/ADT/SmallString.h>
#include <unistd.h>

#include <chrono>
#include <dbg/Log.hpp>
#include <dbg/Logger.hpp>
#include <execution>
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

/// Settings
constexpr auto max_procs{16};

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
    llvm::SmallString<3> threads;
    llvm::SmallString<8> mem_total; // GB
};

auto get_info() -> auto {
    StaticInfo info;
    string buf;
    {
        ifstream is{"/proc/cpuinfo"};
        skip_lines(is, 4);
        info.name = get_string(is);
        skip_lines(is, 5);
        info.threads = get_string(is);
    }
    {
        ifstream is{"/proc/meminfo"};
        info.mem_total = ftos<1>(get<unsigned long>(is) /* KB */ * 1e-6L) + "GB";
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
    int free;
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
};

struct Procs {
    llvm::SmallVector<Proc, 512> procs;
};

auto is_number(string &&s) {
    return all_of(s.begin(), s.end(), [](auto c) { return '0' <= c && c <= '9'; });
}

auto parse_procs(Procs &procs) -> void {
    using ranges::views::filter;

    const auto size{[] {
        const auto di{directory_iterator("/proc")};
        return distance(begin(di), end(di));
    }()};
    procs.procs.resize(size);
    const auto di{directory_iterator("/proc")};
    const auto paths{llvm::iterator_range(begin(di), end(di))};
    const auto zipped{zip(procs.procs, paths)};

    for_each(execution::par, zipped.begin(), zipped.end(), [](auto pair) {
        auto [proc, d]{pair};
        if (!d.is_directory()) {
            return;
        }
        if (!is_number(d.path().stem().string())) {
            return;
        }

        const auto &path{d.path()};
        ifstream is{path / "stat"};
        get(is, proc.pid); // First data is PID

        /// Skip to usage data.
        skip_to(is, ')');
        get(is, proc.state);
        skip_fields<' '>(is, 11);
        proc.usage = 0;
        for (auto i{0}; i < 4; i++) { // The next four fields are usage times.
            proc.usage += get<unsigned long>(is);
        }
    });
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