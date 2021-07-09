/// @file NVML.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include "fprd/util/ranges.hpp"
#include </opt/cuda/targets/x86_64-linux/include/nvml.h>

#include <condition_variable>
#include <dbg/Log.hpp>
#include <fprd/util/ostream.hpp>
#include <fprd/util/time.hpp>
#include <fprd/util/to_string.hpp>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallString.h>
#include <mutex>
#include <thread>
#include <type_traits>

namespace fprd {
using namespace std;

namespace probe {
namespace nvml {
/// Settings
/// Maximum numbers of active processes shown.
constexpr auto max_procs{8};

namespace device {
using ID = nvmlDevice_t;

/// Information about the device that only needs to be obtained once.
struct StaticInfo {
    llvm::SmallString<32> name;
    llvm::SmallString<8> mem_total; // GB
};

auto get_info(ID id) -> auto {
    StaticInfo di;

    nvmlDeviceGetName(id, di.name.data(), 32);

    nvmlMemory_t m;
    nvmlDeviceGetMemoryInfo(id, &m);
    const auto mem_total{m.total * 1e-9L};
    di.mem_total = ftos<3>(mem_total);

    return di;
}

/// Information about the devices that changes frequently.
struct DynamicInfo {
    nvmlUtilization_t util; // %
    nvmlMemory_t mem;       // All in bytes
    unsigned int fan;       // %
    unsigned int temp;      // Celsius
    unsigned int clock;     // MHz
    unsigned int power;     // mW
    llvm::SmallVector<nvmlProcessInfo_t, max_procs> procs;
};

auto update_data(DynamicInfo &data, ID id) -> void {
    nvmlDeviceGetUtilizationRates(id, &data.util);
    nvmlDeviceGetMemoryInfo(id, &data.mem);
    nvmlDeviceGetFanSpeed(id, &data.fan);
    nvmlDeviceGetTemperature(id, NVML_TEMPERATURE_GPU, &data.temp);
    nvmlDeviceGetClockInfo(id, NVML_CLOCK_GRAPHICS, &data.clock);
    nvmlDeviceGetPowerUsage(id, &data.power);

    unsigned int proc_count{0};
    nvmlDeviceGetGraphicsRunningProcesses_v2(id, &proc_count, nullptr);
    if (proc_count < max_procs) {
        data.procs.resize(proc_count);
        nvmlDeviceGetGraphicsRunningProcesses_v2(id, &proc_count, data.procs.data());
    } else {
        data.procs.resize(max_procs);
        vector<nvmlProcessInfo_t> procs(proc_count);
        nvmlDeviceGetGraphicsRunningProcesses_v2(id, &proc_count, procs.data());
        copy(procs.begin(), procs.begin() + max_procs, back_inserter(data.procs));
    }
}
} // namespace device
}; // namespace nvml

/// Auto shutting down NVML instance.
struct NVML {
    NVML() { nvmlInit_v2(); }
    ~NVML() { nvmlShutdown(); }
    /// Diable copy & move because this is supposed to be a singleton.
    NVML(const NVML &) = delete;

    [[nodiscard]] auto get_devices() {
        unsigned int c;
        nvmlDeviceGetCount(&c);
        vector<nvmlDevice_t> devs(c);
        for (auto [idx, d] : devs | enumerate) {
            nvmlDeviceGetHandleByIndex_v2(idx, &d);
        }
        return devs;
    }
};
} // namespace probe
}; // namespace fprd
