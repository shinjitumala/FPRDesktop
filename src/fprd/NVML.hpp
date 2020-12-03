/// @file NVML.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include </opt/cuda/targets/x86_64-linux/include/nvml.h>

#include <condition_variable>
#include <dbg/Log.hpp>
#include <fprd/System.hpp>
#include <fprd/util/ostream.hpp>
#include <mutex>
#include <thread>

namespace fprd {
using namespace std;

namespace nvml {

/// Crash the program if there is an error.
/// @param ret
void check(nvmlReturn_t ret, source_location l = source_location::current()) {
    if (ret != NVML_SUCCESS) {
        fatal_error("NVML ERROR : " << nvmlErrorString(ret) << " at " << l);
    }
};

struct NVML;

/// A GPU device
class Device {
    friend NVML;

    /// Our representation of a process.
    struct Process {
        string name;
        nvmlProcessInfo_t t;
    };

   public:
    /// Maximum number of processes shown.
    inline static constexpr u_char max_procs{8};
    /// Data update interval.
    inline static constexpr auto interval{1s};

    struct DynamicData {
        u_char utilization;         // %
        float memory;               // GB
        u_char utilization_memory;  // %
        u_char fan;                 // %
        u_char temp;                // Celsius
        float power;                // Watts
        ushort clock;               // MHz

        /// Sorted list of processes.
        /// INFO: Maximum of 'max_procs' items shown.
        vector<Process> procs;
    };

    atomic<bool> &running;  // Set to false when stopping.

   private:
    /// The wrapped thing.
    nvmlDevice_t t;

    /// The mutex for 'data'
    unique_ptr<mutex> m;
    /// The dynamically updated data.
    DynamicData data;
    /// The data update thread.
    thread updater;
    /// Set to true when there is new data available.
    unique_ptr<atomic<bool>> update_available;

   public:
    const string name;         // Product name
    const float memory_total;  // GB

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

    Device(atomic<bool> &running, nvmlDevice_t t)
        : running{running},
          t{t},
          m{make_unique<mutex>()},
          data{},
          updater{[this]() { Device::update_loop(*this); }},
          update_available{make_unique<atomic<bool>>(false)},
          name{[t]() -> string {
              array<char, 96> buf;
              nvmlDeviceGetName(t, buf.data(), buf.size());
              return buf.data();
          }()},
          memory_total{[t]() -> float {
              nvmlMemory_t m;
              check(nvmlDeviceGetMemoryInfo(t, &m));
              return static_cast<float>((float)m.total * 1e-9);
          }()} {}

    ~Device() { updater.join(); }

    Device(const Device &) = delete;
    Device(Device &&) = default;

   private:
    static void update_loop(Device &d) {
        while (d.running) {
            using namespace ::std::chrono;
            const auto t{time_point<high_resolution_clock>::clock::now()};
            {
                lock_guard lg{*d.m};
                d.data = update(d);
                *d.update_available = true;
            }
            this_thread::sleep_until(t + interval);
        }
    }

    /// Update mutable data for this device.
    static DynamicData update(const Device &d) {
        dbg(auto tp{dbg::now()});

        const auto &t{d.t};
        DynamicData data;
        tie(data.utilization, data.utilization_memory) = [t]() {
            nvmlUtilization_t u;
            check(nvmlDeviceGetUtilizationRates(t, &u));
            return make_pair(u.gpu, u.memory);
        }();
        data.memory = [t]() {
            nvmlMemory_t m;
            check(nvmlDeviceGetMemoryInfo(t, &m));
            return (float)m.used * 1e-9;
        }();
        data.fan = [t]() {
            unsigned int s;
            check(nvmlDeviceGetFanSpeed(t, &s));
            return s;
        }();
        data.temp = [t]() {
            unsigned int temp;
            check(nvmlDeviceGetTemperature(t, NVML_TEMPERATURE_GPU, &temp));
            return temp;
        }();
        data.power = [t]() {
            unsigned int p;
            check(nvmlDeviceGetPowerUsage(t, &p));
            return (float)p / 1000;
        }();
        data.clock = [t]() {
            unsigned int c;
            check(nvmlDeviceGetClockInfo(t, NVML_CLOCK_GRAPHICS, &c));
            return c;
        }();
        data.procs = [t]() {
            const auto procs{[t]() {
                auto procs{[t]() -> vector<nvmlProcessInfo_t> {
                    array<nvmlProcessInfo_t, 16> i;
                    unsigned int c{i.size()};
                    check(nvmlDeviceGetGraphicsRunningProcesses_v2(t, &c,
                                                                   i.data()));

                    sort(i.begin(), i.begin() + c, [](auto &l, auto &r) {
                        return l.usedGpuMemory > r.usedGpuMemory;
                    });
                    if (c > max_procs) {
                        return {i.begin(), i.begin() + 5};
                    }
                    return {i.begin(), i.begin() + c};
                }()};
                return procs;
            }()};
            vector<Process> ps;
            ps.reserve(ps.size());
            for (auto p : procs) {
                ps.emplace_back(Process{get_name(p.pid), p});
            }
            return ps;
        }();
        dbg_out("GPU data: " << dbg::diff(tp) << "ms");
        return data;
    }
};

/// Auto shutting down NVML instance.
struct NVML {
   private:
    [[nodiscard]] auto get_device_count() const {
        unsigned int c;
        check(nvmlDeviceGetCount(&c));
        return c;
    }

   public:
    NVML() { check(nvmlInit_v2()); }
    ~NVML() { check(nvmlShutdown()); }
    NVML(const NVML &) = delete;

    [[nodiscard]] auto get_devices(atomic<bool> &run) const {
        const auto c{get_device_count()};
        vector<Device> devs;
        devs.reserve(c);
        for (auto i{0U}; i < c; i++) {
            nvmlDevice_t d;
            check(nvmlDeviceGetHandleByIndex_v2(i, &d));
            devs.emplace_back(run, d);
        }
        return devs;
    }
};

};  // namespace nvml
};  // namespace fprd
