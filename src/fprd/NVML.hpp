/// @file NVML.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include </opt/cuda/targets/x86_64-linux/include/nvml.h>

#include <dbg/Log.hpp>
#include <fprd/System.hpp>
#include <fprd/util/ostream.hpp>
#include <span>

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
    // Maximum number of processes shown.
    inline static constexpr u_char max_procs{5};

   private:
    /// The wrapped thing.
    nvmlDevice_t t;

   public:
    const string name;          // Product name
    const ushort memory_total;  // GB

    u_char utilization;  // %
    u_char memory;       // %
    u_char fan;          // %
    u_char temp;         // Celsius
    float power;         // Watts
    short clock;         // MHz

    /// Sorted list of processes.
    /// INFO: Maximum of 'max_procs' items shown.
    vector<Process> procs;

   private:
    Device(nvmlDevice_t t)
        : t{t},
          name{[t]() -> string {
              array<char, 96> buf;
              nvmlDeviceGetName(t, buf.data(), buf.size());
              return buf.data();
          }()},
          memory_total{[t]() -> ushort {
              nvmlMemory_t m;
              check(nvmlDeviceGetMemoryInfo(t, &m));
              return static_cast<ushort>((double)m.total * 1e-9);
          }()} {};

   public:
    /// Update mutable data for this device.
    void update() {
        tie(utilization, memory) = [this]() {
            nvmlUtilization_t u;
            check(nvmlDeviceGetUtilizationRates(t, &u));
            return pair<u_char, u_char>{u.gpu, u.memory};
        }();
        fan = [this]() {
            unsigned int s;
            check(nvmlDeviceGetFanSpeed(t, &s));
            return s;
        }();
        temp = [this]() {
            unsigned int temp;
            check(nvmlDeviceGetTemperature(t, NVML_TEMPERATURE_GPU, &temp));
            return temp;
        }();
        power = [this]() {
            unsigned int p;
            check(nvmlDeviceGetPowerUsage(t, &p));
            return (float)p / 1000;
        }();
        clock = [this]() {
            unsigned int c;
            check(nvmlDeviceGetClockInfo(t, NVML_CLOCK_GRAPHICS, &c));
            return c;
        }();
        procs = [this]() {
            const auto procs{[this]() {
                auto procs{[this]() -> vector<nvmlProcessInfo_t> {
                    array<nvmlProcessInfo_t, 16> i;
                    unsigned int c{i.size()};
                    check(nvmlDeviceGetGraphicsRunningProcesses_v2(t, &c,
                                                                   i.data()));
                    if (c > max_procs) {
                        return {i.begin(), i.begin() + 5};
                    }
                    return {i.begin(), i.begin() + c};
                }()};
                sort(procs.begin(), procs.end(), [](auto &l, auto &r) {
                    return l.usedGpuMemory > r.usedGpuMemory;
                });
                return procs;
            }()};
            vector<Process> ps;
            ps.reserve(ps.size());
            for (auto p : procs) {
                ps.emplace_back(Process{get_name(p.pid), p});
            }
            return ps;
        }();
    }
};

/// Auto shutting down NVML instance.
struct NVML {
    NVML() { check(nvmlInit_v2()); }
    ~NVML() { check(nvmlShutdown()); }
    NVML(const NVML &) = delete;

    [[nodiscard]] auto get_device_count() const {
        unsigned int c;
        check(nvmlDeviceGetCount(&c));
        return c;
    }

    [[nodiscard]] auto get_device_handle(size_t index) const {
        nvmlDevice_t d;
        check(nvmlDeviceGetHandleByIndex_v2(index, &d));
        return Device(d);
    }
};

};  // namespace nvml
};  // namespace fprd
