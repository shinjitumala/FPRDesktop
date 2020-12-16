/// @file cpuinfo.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <array>
#include <dbg/Log.hpp>
#include <fprd/unix/is_utils.hpp>
#include <fstream>

namespace fprd {
auto getval(istream& is) {
    const auto l{getline(is)};
    const auto itr{l.find(':')};
    return l.substr(itr + 1, numeric_limits<size_t>::max());
}

/// Get basic CPU info.
/// @return auto CPU name and thread count.
auto get_cpu_info() {
    ifstream is{"/proc/cpuinfo"};
    skip_lines(is, 4);
    const auto name{getval(is)};
    skip_lines(is, 5);
    const auto thread_count{stoul(getval(is))};
    return make_pair(name, thread_count);
}

/// WARNING: This usage info is the value "since the beginning".
struct UsageInfo {
    ulong total;
    ulong idle;
};

template <size_t thread_count>
struct ThreadInfo {
    UsageInfo overall;
    array<UsageInfo, thread_count> threads;
};

template <size_t thread_count>
auto get_thread_info() {
    ifstream is{"/proc/stat"};

    /// @return auto idle and total time.
    auto parse_line{[&]() {
        skip_to(is, ' ');  // Skip CPU name.
        ulong total{0};
        ulong idle{0};
        for (auto i{0}; i < 10; i++) {
            const auto l{getulong(is)};
            if (i == 3) {
                // The 3rd number is the total idle time.
                idle += l;
            }
            total += l;
        }
        skip_lines(is, 1);  // Skip the rest of the current line.
        return UsageInfo{total, idle};
    }};

    return ThreadInfo<thread_count>{
        .overall = parse_line(),
        .threads =
            [&] {
                array<UsageInfo, thread_count> r;
                for (auto i{0}; i < r.size(); i++) {
                    r[i] = parse_line();
                }
                return r;
            }(),
    };
}

/// @tparam thread_count
/// @return auto Frequency of each thread in MHz
template <size_t thread_count>
auto get_freq_info() {
    ifstream is{"/proc/cpuinfo"};
    skip_lines(is, 7);  // Skip first two lines.
    array<float, thread_count> freqs;
    for (auto& freq : freqs) {
        freq = stof(getval(is));
        skip_lines(is, 27);
    }
    return freqs;
}
};  // namespace fprd