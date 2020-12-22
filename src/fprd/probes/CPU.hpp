/// @file CPU.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <unistd.h>

#include <chrono>
#include <dbg/Log.hpp>
#include <dbg/Logger.hpp>
#include <filesystem>
#include <fprd/util/istream.hpp>
#include <fprd/util/ranges.hpp>
#include <fprd/util/time.hpp>
#include <fprd/util/to_string.hpp>
#include <fstream>
#include <future>

namespace fprd {
using namespace ::std;
using namespace ::std::filesystem;

namespace probe {

/// Utility function for parsing values from UNIX human-readable files.
/// @param is
/// @return auto
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

/// UsageInfo for the entire CPU.
struct CPUUsage {
    /// WARNING: This usage info is the value "since the beginning".
    struct Usage {
        ulong total;
        ulong idle;
    };

    Usage overall;
    vector<Usage> threads;
};

/// Obtains raw CPU usage info.
/// The raw CPU usage info shows the LIFETIME usage of the CPU.
/// This means that to compute the CURRENT usage, we must compute the delta.
/// This function obtains the LIFETIME usage.
/// @param thread_count
/// @return CPUUsageInfo
auto get_cpu_lifetime_usage(size_t thread_count) -> CPUUsage {
    ifstream is{"/proc/stat"};

    /// @return auto idle and total time.
    auto parse_line{[&]() -> CPUUsage::Usage {
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
        return {total, idle};
    }};

    // First line is the average of all threads (cores).
    const auto overall{parse_line()};
    // Beyond the first line is all the individual threads (cores).
    const auto threads{[&] {
        vector<CPUUsage::Usage> r;
        for (auto i{0}; i < r.size(); i++) {
            r[i] = parse_line();
        }
        return r;
    }()};

    return {overall, threads};
}

/// Get the current frequencies of the entire CPU.
/// @tparam thread_count
/// @return auto Frequency of each thread in MHz
auto get_cpu_freqs(size_t thread_count) {
    ifstream is{"/proc/cpuinfo"};
    skip_lines(is, 7);  // Skip first two lines.
    vector<float> freqs;
    for (auto& freq : freqs) {
        freq = stof(getval(is));
        skip_lines(is, 27);
    }
    return freqs;
}

/// For querying CPU related stuff.
/// @tparam max_procs Maximum number of processes shown.
template <size_t max_procs>
class CPU {
    /// All information about a thread at a certain moment.
    struct ThreadStatus {
        float usage;  // %
        float freq;   // GHz
    };

    /// For tracking the usage of processes.
    struct BasicProcess {
        pid_t pid;
        long use;  // lifetime usage.
    };

    /// Used when sorting processes.
    struct ProcessUsage : public BasicProcess {
        float usage;  // % current usage.
    };

   public:
    /// Only created for processes that are above 0% usage.
    struct Process : public ProcessUsage {
        string name;
        char mode;
        ushort mem;  // MB

        ostream& print(ostream& os) const {
            os << "{ ";
            {
                dbg::IndentGuard ig{};
                os << "pid: " << this->pid << ", ";
                os << "name: " << name << ", ";
                os << "mode: " << mode << ", ";
                os << "usage: " << this->usage << ", ";
                os << "mem: " << mem << ", ";
            }
            os << "}";
            return os;
        }
    };

    struct DynamicData {
        vector<ThreadStatus> threads;  // Hard-coded thread count
        ThreadStatus avg;              // Average usage and frequencies.
        u_char temp;                   // Celsius

        vector<Process> procs;  // Processes.
    };

    const string name;
    const size_t thread_count;

    /// Save the LIFETIME usage for all processes in the system.
    /// This is needed to compute the CURRENT usage.
    /// See 'get_cpu_lifetime_usage' for a more detailed explanation.
    vector<BasicProcess> tracked_procs;

    CPU() : CPU{get_cpu_info()} {}
    CPU(const CPU&) = delete;

    [[nodiscard]] auto update() {
        dbg(const auto tp{now()});

        DynamicData data;

        const auto ti{get_cpu_freqs(thread_count)};

        data.temp = [] {
            // Ad-hoc way of getting temperatures in FPR's machine in Dec. 2020.
            // Masu, fuck you btw.
            ifstream is{"/sys/class/thermal/thermal_zone2/temp"};
            return getulong(is) / 1000;
        }();
        data.procs = read_proc(1000);

        dbg_out("CPU data: " << diff(tp) << "ms");
        return data;
    }

   private:
    CPU(pair<string, size_t> name_thread_count)
        : name{name_thread_count.first},
          thread_count{name_thread_count.second} {}

    auto is_number(string_view s) {
        return any_of(s.begin(), s.end(),
                      [](auto c) { return c < '0' || '9' < c; });
    }

    auto read_proc(unsigned long current_cpu_usage) {
        const auto sorted{[&] {
            vector<ProcessUsage> procs;
            for (const auto& d : directory_iterator("/proc")) {
                // Skip non-directories
                if (!d.is_directory()) {
                    continue;
                }
                // Skip directories that are not PIDs (number).
                const auto& p{d.path()};
                if (!is_number(p.stem().string())) {
                    continue;
                }

                ifstream is{p / "stat"};
                // First data is the PID.
                pid_t pid{getint(is)};

                auto& bproc{
                    [&is, pid, this]()
                        -> auto& {
                            if (const auto itr{find_if(
                                    tracked_procs.begin(), tracked_procs.end(),
                                    [pid](auto& p) { return p.pid == pid; })};
                                itr != tracked_procs.end()){return *itr;
            }

            // Insert a new basic proc.
            return tracked_procs.emplace_back(pid, 0);
        }()};

        /// Skip to usage data.
        skip_to(is, ')');
        for (auto i{0}; i < 13; i++) {
            skip_to(is, ' ');
        }

        const auto last_use{bproc.use};
        bproc.use = [&]() {
            ulong sum{};
            sum += getulong(is);
            sum += getulong(is);
            sum += getulong(is);
            sum += getulong(is);
            return sum;
        }();

        ProcessUsage usage{bproc};
        usage.usage = (bproc.use - last_use) / current_cpu_usage * 100;

        /// Skip if usage is 0.
        if (usage.usage == 0) {
            continue;
        }

        procs.push_back(usage);
    }

    // Sort the processes by usage.
    sort(procs.begin(), procs.end(),
         [&](auto& lhs, auto& rhs) { return lhs.usage > rhs.usage; });

    // Shrink the vector to the maximum size if needed.
    if (procs.size() > max_procs) {
        return vector<ProcessUsage>(procs.begin(), procs.begin() + max_procs);
    }
    return procs;
}()
};

vector<Process> procs;
for (auto p : sorted) {
    Process proc{p};
    ifstream is{"/proc/" + to_string(p.pid) + "/stat"};
    skip_to(is, ')');
    proc.mode = getchar(is);
    for (auto i{0}; i < 14; i++) {
        skip_to(is, ' ');
    }
    for (auto i{0}; i < 7; i++) {
        skip_to(is, ' ');
    }
    proc.mem = static_cast<float>((double)getulong(is) * 1e-6 *
                                  (double)::sysconf(_SC_PAGE_SIZE));

    procs.push_back(proc);
}
return procs;
}
}
;
}
;  // namespace probe
}
;  // namespace fprd