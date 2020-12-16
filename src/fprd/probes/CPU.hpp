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
#include <fprd/unix/cpuinfo.hpp>
#include <fprd/util/ranges.hpp>
#include <fprd/util/time.hpp>
#include <fprd/util/to_string.hpp>
#include <fstream>
#include <future>

namespace fprd {
using namespace ::std;
using namespace ::std::filesystem;

namespace query {

/// For querying CPU related stuff.
class CPU {
   public:
    inline static const u_char max_procs{16};
    /// Data update interval.
    inline static constexpr auto interval{1s};
    /// The hard-coded thread count
    inline static constexpr auto thread_count{16};

   private:
    struct ThreadStatus {
        UsageInfo usage_value{};
        float usage;  // %
        float freq;   // GHz

        auto update(const ThreadStatus& old, UsageInfo new_values) {
            const auto d_total{new_values.total - old.usage_value.total};
            const auto d_idle{new_values.idle - old.usage_value.idle};

            usage = [=]() -> float {
                const auto d_use{d_total - d_idle};
                if (d_use == 0 || d_total == 0) {
                    return 0.0F;
                }
                return (float)(d_total - d_idle) / d_total * 100;
            }();
            usage_value = new_values;
            return d_total;
        }

        ostream& print(ostream& os) const {
            os << "{" << usage_value.total << ", " << usage_value.idle << "}";
            return os;
        }
    };

    /// For tracking the usage of processes.
    struct BasicProcess {
        unsigned int pid;
        long use{};  // Usage (since the begninning)
    };

    /// Only created for processes that are above 0% usage.
    struct Process : public BasicProcess {
        string name;
        char mode;
        float usage;  // % current usage.
        ushort mem;   // MB

        ostream& print(ostream& os) const {
            os << "{ ";
            {
                dbg::IndentGuard ig{};
                os << "pid: " << pid << ", ";
                os << "name: " << name << ", ";
                os << "mode: " << mode << ", ";
                os << "usage: " << usage << ", ";
                os << "mem: " << mem << ", ";
            }
            os << "}";
            return os;
        }
    };

   public:
    struct DynamicData {
        array<ThreadStatus, thread_count> threads;  // Hard-coded thread count
        ThreadStatus avg;  // Average usage and frequencies.
        u_char temp;       // Celsius

        vector<BasicProcess> uses;        // Save the usage for all processes.
        array<Process, max_procs> procs;  // Processes.
    };

    const string name;

   private:
    /// Set to false when stopping.
    atomic<bool>& running;
    /// The mutex for 'data'
    unique_ptr<mutex> m;
    /// The dynamically updated data.
    DynamicData data;
    /// The data update thread.
    thread updater;
    /// Set to true when there is new data available.
    unique_ptr<atomic<bool>> update_available;

   public:
    CPU(atomic<bool>& running)
        : name{[] {
              auto [name, thread_count]{get_cpu_info()};
              if (thread_count != CPU::thread_count) {
                  fatal_error("Hard-coded thread count is incorrect. Real: "
                              << thread_count
                              << ", Hard-coded: " << CPU::thread_count);
              }
              return name;
          }()},
          running{running},
          m{make_unique<mutex>()},
          updater{&CPU::update_loop, this},
          update_available{make_unique<atomic<bool>>(false)} {
        // Initial check
        if (!exists("/sys/class/thermal/thermal_zone2/temp")) {
            fatal_error("File missing: /sys/class/thermal/thermal_zone2/temp");
        }

        lock_guard ig{*m};
        // Must be initialized at first run.
        data = update();
        *update_available = true;
    }
    CPU(const CPU&) = delete;
    ~CPU() { updater.join(); }

    /// Checks if there is new data and consumes it if there is one.
    /// @return auto
    [[nodiscard]] auto get_dynamic_data() {
        if (*update_available) {
            *update_available = false;
            lock_guard lg{*m};
            return make_pair(true, data);
        }
        return make_pair(false, DynamicData{});
    }

   private:
    void update_loop() {
        while (running) {
            const auto t{now()};
            {
                lock_guard lg{*m};
                data = update();
                *update_available = true;
            }
            this_thread::sleep_until(t + interval);
        }
    }

    [[nodiscard]] DynamicData update() const {
        dbg(const auto tp{now()});
        DynamicData new_data;
        const auto ti{get_thread_info<thread_count>()};
        const auto current_cycles{new_data.avg.update(data.avg, ti.overall)};
        for (auto [nt, ot, tu, fq] :
             zip(new_data.threads, data.threads, ti.threads,
                 get_freq_info<thread_count>())) {
            nt.update(ot, tu);
            nt.freq = fq;
        }

        new_data.temp = [] {
            ifstream is{"/sys/class/thermal/thermal_zone2/temp"};
            return getulong(is) / 1000;
        }();
        tie(new_data.procs, new_data.uses) = [&]() {
            path proc{"/proc/"};
            vector<Process> procs;
            vector<BasicProcess> new_uses;
            for (const auto& d : directory_iterator(proc)) {
                // Skip non-directories
                if (!d.is_directory()) {
                    continue;
                }
                // Skip directories that are not PIDs (number).
                const auto& path{d.path()};
                if ([&]() {
                        const auto s{path.stem().string()};
                        return any_of(s.begin(), s.end(), [](auto c) {
                            return c < '0' || '9' < c;
                        });
                    }()) {
                    continue;
                }

                ifstream is{path / "stat"};
                /// Find the last usage data if it exists, otherwise create a
                /// fresh one.
                const auto last_use{[&is, &uses = data.uses]() -> BasicProcess {
                    const auto pid{getuint(is)};
                    if (const auto itr{
                            find_if(uses.begin(), uses.end(),
                                    [pid](auto& u) { return u.pid == pid; })};
                        itr != uses.end()) {
                        return *itr;
                    }
                    return {pid, 0};
                }()};
                BasicProcess new_use{.pid = last_use.pid};

                /// Skip to usage data.
                skip_to(is, ')');
                for (auto i{0}; i < 13; i++) {
                    skip_to(is, ' ');
                }
                new_use.use = [&]() {
                    ulong sum{};
                    sum += getulong(is);
                    sum += getulong(is);
                    sum += getulong(is);
                    sum += getulong(is);
                    return sum;
                }();
                const auto d_use{new_use.use - last_use.use};
                new_uses.push_back(new_use);
                if (d_use == 0) {
                    // Ignore if 0 usage this interval.
                    continue;
                }

                procs.push_back({[&]() {
                    Process p{new_use};

                    // Parase other information
                    is.seekg(ios_base::beg);

                    skip_to(is, '(');
                    p.name = [&]() {
                        string name;
                        for (char c; is >> c, is && c != ')';) {
                            name += c;
                        }
                        return name;
                    }();
                    p.mode = getchar(is);
                    for (auto i{0}; i < 14; i++) {
                        skip_to(is, ' ');
                    }
                    p.usage = [&]() {
                        if (last_use.use == 0) {
                            return 0.0f;
                        }
                        return (float)d_use / current_cycles;
                    }();
                    for (auto i{0}; i < 7; i++) {
                        skip_to(is, ' ');
                    }
                    p.mem =
                        static_cast<float>((double)getulong(is) * 1e-6 *
                                           (double)::sysconf(_SC_PAGE_SIZE));
                    return p;
                }()});
            }
            sort(procs.begin(), procs.end(),
                 [&](auto& lhs, auto& rhs) { return lhs.usage > rhs.usage; });
            array<Process, max_procs> new_procs;
            if (procs.size() > max_procs) {
                copy(procs.begin(), procs.begin() + max_procs,
                     new_procs.begin());
            } else {
                copy(procs.begin(), procs.end(), new_procs.begin());
            }
            return make_pair(new_procs, new_uses);
        }();

        dbg_out("CPU data: " << diff(tp) << "ms");
        return new_data;
    }
};
};  // namespace query
};  // namespace fprd