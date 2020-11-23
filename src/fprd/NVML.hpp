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
#include <future>
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
    const string name;         // Product name
    const float memory_total;  // GB

    struct DynamicData {
        u_char utilization;         // %
        u_char memory;              // %
        u_char utilization_memory;  // %
        u_char fan;                 // %
        u_char temp;                // Celsius
        float power;                // Watts
        short clock;                // MHz

        /// Sorted list of processes.
        /// INFO: Maximum of 'max_procs' items shown.
        vector<Process> procs;
    };
    DynamicData data;
    future<DynamicData> f;

   private:
    Device(nvmlDevice_t t)
        : t{t},
          name{[t]() -> string {
              array<char, 96> buf;
              nvmlDeviceGetName(t, buf.data(), buf.size());
              return buf.data();
          }()},
          memory_total{[t]() -> float {
              nvmlMemory_t m;
              check(nvmlDeviceGetMemoryInfo(t, &m));
              return static_cast<float>((float)m.total * 1e-9);
          }()} {};

   public:
    void start_update() {
        promise<DynamicData> p;
        f = async(launch::async, &Device::update, this);
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
    /// Update mutable data for this device.
    [[nodiscard]] DynamicData update() const {
        DynamicData data;
        tie(data.utilization, data.utilization_memory) = [this]() {
            nvmlUtilization_t u;
            check(nvmlDeviceGetUtilizationRates(t, &u));
            return make_pair(u.gpu, u.memory);
        }();
        data.memory = [this]() {
            nvmlMemory_t m;
            check(nvmlDeviceGetMemoryInfo(t, &m));
            return static_cast<u_char>((double)m.used / (double)m.total * 100);
        }();
        data.fan = [this]() {
            unsigned int s;
            check(nvmlDeviceGetFanSpeed(t, &s));
            return s;
        }();
        data.temp = [this]() {
            unsigned int temp;
            check(nvmlDeviceGetTemperature(t, NVML_TEMPERATURE_GPU, &temp));
            return temp;
        }();
        data.power = [this]() {
            unsigned int p;
            check(nvmlDeviceGetPowerUsage(t, &p));
            return (float)p / 1000;
        }();
        data.clock = [this]() {
            unsigned int c;
            check(nvmlDeviceGetClockInfo(t, NVML_CLOCK_GRAPHICS, &c));
            return c;
        }();
        data.procs = [this]() {
            const auto procs{[this]() {
                auto procs{[this]() -> vector<nvmlProcessInfo_t> {
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

    [[nodiscard]] auto get_devices() const {
        const auto c{get_device_count()};
        vector<Device> devs;
        devs.reserve(c);
        for (auto i{0U}; i < c; i++) {
            nvmlDevice_t d;
            check(nvmlDeviceGetHandleByIndex_v2(i, &d));
            devs.push_back(Device{d});
        }
        return devs;
    }
};

};  // namespace nvml
};  // namespace fprd
