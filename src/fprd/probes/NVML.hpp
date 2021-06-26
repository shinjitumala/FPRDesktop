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
#include <fprd/probes/UNIX.hpp>
#include <fprd/util/ostream.hpp>
#include <fprd/util/time.hpp>
#include <fprd/util/to_string.hpp>
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
/// @tparam max_procs Maximum number of processes shown.
template <u_char max_procs> class Device {
    friend NVML;

  public:
    /// Our representation of a process.
    struct Process {
        string name;
        nvmlProcessInfo_t t;

        static constexpr auto name_size{32};
        static constexpr auto pid_size{7};
        static constexpr auto memory_size{8};

        static string header() {
            ostringstream os;
            os << setfill(' ') << setw(pid_size) << right << "PID";
            os << " ";
            os << setfill(' ') << setw(name_size) << left << "Name";
            os << " ";
            os << setfill(' ') << setw(memory_size) << right << "Memory";

            return os.str();
        };

        bool operator==(const Process &rhs) const { return t.pid == rhs.t.pid; }

        ostream &print(ostream &os) const {
            os << setfill(' ') << setw(pid_size) << right << t.pid;
            os << " ";
            os << setfill(' ') << setw(name_size) << left << truncs<name_size>(name);
            os << " ";
            os << setfill(' ') << setw(memory_size) << right << (ftos<0>(t.usedGpuMemory / 1000000.0F) + "MB");
            return os;
        };
    };

    struct DynamicData {
        u_char utilization;        // %
        float memory;              // GB
        u_char utilization_memory; // %
        u_char fan;                // %
        u_char temp;               // Celsius
        float power;               // Watts
        ushort clock;              // MHz

        /// Sorted list of processes.
        /// INFO: Maximum of 'max_procs' items shown.
        vector<Process> procs;
    };

  private:
    /// The wrapped thing.
    nvmlDevice_t t;

  public:
    const string name;        // Product name
    const float memory_total; // GB

    Device(nvmlDevice_t t)
        : t{t}, name{[t]() -> string {
              array<char, 96> buf;
              nvmlDeviceGetName(t, buf.data(), buf.size());
              return buf.data();
          }()},
          memory_total{[t]() -> float {
              nvmlMemory_t m;
              check(nvmlDeviceGetMemoryInfo(t, &m));
              return static_cast<float>((float)m.total * 1e-9);
          }()} {}

    Device(const Device &) = delete;
    Device(Device &&) noexcept = default;

    /// Update mutable data for this device.
    [[nodiscard]] DynamicData probe_data() const {
        dbg(auto tp{now()});

        DynamicData data;
        tie(data.utilization, data.utilization_memory) = [this]() {
            nvmlUtilization_t u;
            check(nvmlDeviceGetUtilizationRates(t, &u));
            return make_pair(u.gpu, u.memory);
        }();
        data.memory = [this]() {
            nvmlMemory_t m;
            check(nvmlDeviceGetMemoryInfo(t, &m));
            return (float)m.used * 1e-9;
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
                    check(nvmlDeviceGetGraphicsRunningProcesses_v2(t, &c, i.data()));

                    sort(i.begin(), i.begin() + c,
                         [](auto &l, auto &r) { return l.usedGpuMemory > r.usedGpuMemory; });
                    if (c > max_procs) {
                        return {i.begin(), i.begin() + max_procs};
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
        dbg_out("GPU data: " << diff(tp) << "ms");
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

    template <u_char max_procs> [[nodiscard]] auto get_devices() const {
        const auto c{get_device_count()};
        vector<Device<max_procs>> devs;
        devs.reserve(c);
        for (auto i{0U}; i < c; i++) {
            nvmlDevice_t d;
            check(nvmlDeviceGetHandleByIndex_v2(i, &d));
            devs.emplace_back(d);
        }
        return devs;
    }
};

}; // namespace nvml
}; // namespace fprd
