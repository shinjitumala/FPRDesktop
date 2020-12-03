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
#include <fprd/util/to_string.hpp>
#include <fstream>
#include <future>

namespace fprd {
using namespace ::std;
using namespace ::std::filesystem;

namespace query {
auto& skip_lines(istream& is, ushort lines) {
    for (auto i{0U}; i < lines; i++) {
        is.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return is;
}

auto getline(istream& is) {
    string s;
    getline(is, s);
    return s;
}

auto getstring(istream& is) {
    string s;
    is >> s;
    return s;
}

auto getval(istream& is) {
    const auto l{getline(is)};
    const auto itr{l.find(':')};
    return l.substr(itr + 1, numeric_limits<size_t>::max());
}

auto getfloat(istream& is) {
    float f;
    is >> f;
    return f;
}

auto getulong(istream& is) {
    ulong l;
    is >> l;
    return l;
}

auto getchar(istream& is) {
    char c;
    is >> c;
    return c;
}

auto skip_to(istream& is, char delim) {
    is.ignore(numeric_limits<streamsize>::max(), delim);
}

/// For querying CPU related stuff.
class CPU {
   public:
    inline static const u_char max_procs{16};

    const string name;
    const u_char thread_count;

   private:
    struct ThreadStatus {
        ulong total;
        ulong idle;
        float usage;  // %
        float freq;   // GHz
    };

    struct Process {
        unsigned int pid;
        string name;
        char mode;
        ulong use;    // Usage (since the beginning)
        float usage;  // % current usage.
        float mem;    // GB

        ostream& print(ostream& os) const {
            os << "{ ";
            {
                dbg::IndentGuard ig{};
                os << "pid: " << pid << ", ";
                os << "name: " << name << ", ";
                os << "mode: " << mode << ", ";
                os << "usage: " << usage << ", ";
                os << "mem: " << ftos<3>(mem) << ", ";
            }
            os << "}";
            return os;
        }
    };

   public:
    struct DynamicData {
        vector<ThreadStatus> threads;
        ulong idle;   // Idle (since the begninning)
        ulong total;  // Total (including idle times, since the beginning)
        ulong total_current;  // The total CPU time since the last update.
        float usage;          // % current usage.
        float freq;           // GHz
        u_char temp;          // Celsius

        vector<Process> procs;
    };
    DynamicData data;
    future<DynamicData> f;

    chrono::time_point<chrono::high_resolution_clock> t;

    CPU()
        : CPU{[&]() {
              ifstream is{"/proc/cpuinfo"};
              skip_lines(is, 4);
              const auto name{getval(is)};
              skip_lines(is, 5);
              const auto count{stoi(getval(is))};
              return make_pair(name, count);
          }()} {}

    CPU(const CPU&) = delete;

    void start_update() {
        if (!f.valid()) {
            t = chrono::time_point<chrono::high_resolution_clock>::clock::now();
            f = async(launch::async, &CPU::update, this, data);
        }
    }

    /// @return true There is new data.
    /// @return false
    bool check_update() {
        if (f.valid() && f.wait_for(0s) == future_status::ready) {
            data = f.get();
            return true;
        }
        return false;
    }

   private:
    CPU(pair<string, u_char> p0)
        : name{[&raw = p0.first]() {
              auto ipos{raw.find('i')};
              auto endpos{raw.find(' ', ipos)};
              return "Intel " + raw.substr(ipos, endpos - ipos);
          }()},
          thread_count{p0.second},
          data{.threads = vector<ThreadStatus>(thread_count)} {}

    [[nodiscard]] DynamicData update(DynamicData last_data) const {
        DynamicData data;
        data.threads = vector<ThreadStatus>(thread_count);
        auto& cores{data.threads};
        {
            // Get CPU usage information.
            ifstream is{"/proc/stat"};
            for (u_char i{0}; i <= thread_count; i++) {
                skip_to(is, ' ');  // Skip the first field, CPU name.
                ulong total{0};
                ulong idle{0};
                for (auto i{0}; i < 10; i++) {
                    const auto l{getulong(is)};
                    if (i == 3) {
                        idle += l;
                    }
                    total += l;
                }

                if (i != 0) {
                    // The second line and beyond are individual threads.
                    const auto ii{i - 1};
                    const auto d{total - last_data.threads[ii].total};
                    const auto d_idle{idle - last_data.threads[ii].idle};
                    cores[ii].usage = (float)(d - d_idle) / d;
                    cores[ii].idle = idle;
                    cores[ii].total = total;
                } else {
                    // The first line is the total CPU
                    const auto d{total - last_data.total};
                    const auto d_idle{idle - last_data.idle};
                    data.usage = (float)(d - d_idle) / d;
                    data.total_current = d;
                    data.idle = idle;
                    data.total = total;
                }
                skip_lines(is, 1);
            }
        }
        {
            ifstream is{"/proc/cpuinfo"};
            float avg{0};
            skip_lines(is, 7);
            for (u_char i{0}; i < thread_count; i++) {
                cores[i].freq = stof(getval(is)) / 1000;
                avg += cores[i].freq / 8;
                skip_lines(is, 27);
            }
            data.freq = avg;
        }
        {
            if (!exists("/sys/class/thermal/thermal_zone2/temp")) {
                fatal_error(
                    "File missing: /sys/class/thermal/thermal_zone2/temp");
            }
            ifstream is{"/sys/class/thermal/thermal_zone2/temp"};
            ushort t;
            is >> t;
            data.temp = t / 1000;
        }
        data.procs = [&]() {
            path proc{"/proc/"};
            vector<Process> procs;
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

                Process p{};
                ifstream is{path / "stat"};
                p.pid = getulong(is);

                // Find the data of this process in the last data.
                const auto last_p{[&]() -> Process {
                    if (auto itr{find_if(
                            last_data.procs.begin(), last_data.procs.end(),
                            [&](auto& lp) { return lp.pid == p.pid; })};
                        itr != last_data.procs.end()) {
                        return *itr;
                    }
                    return {};
                }()};

                skip_to(is, '(');
                p.name = [&]() {
                    string name;
                    for (char c; is >> c, is && c != ')';) {
                        name += c;
                    }
                    return name;
                }();
                p.mode = getchar(is);
                for (auto i{0}; i < 11; i++) {
                    skip_to(is, ' ');
                }
                p.use = [&]() {
                    ulong sum{};
                    sum += getulong(is);
                    sum += getulong(is);
                    sum += getulong(is);
                    sum += getulong(is);
                    return sum;
                }();
                p.usage = [&]() {
                    const auto v{(float)(p.use - last_p.use) /
                                 data.total_current};
                    if (v < 0) {
                        return 0.0f;
                    }
                    if (1 < v) {
                        return 1.0f;
                    }
                    return v;
                }();
                for (auto i{0}; i < 7; i++) {
                    skip_to(is, ' ');
                }

                p.mem = static_cast<float>((double)getulong(is) * 1e-9 *
                                           (double)::sysconf(_SC_PAGE_SIZE));
                procs.push_back(move(p));
            }
            sort(procs.begin(), procs.end(),
                 [&](auto& lhs, auto& rhs) { return lhs.usage > rhs.usage; });
            return procs;
        }();

        return data;
    }
};
};  // namespace query
};  // namespace fprd