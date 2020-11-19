/// @file Nvidia.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <dbg/Logger.hpp>
#include <fprd/System.hpp>

namespace fprd::query {
template <uint gpu_id> struct Nvidia {
    inline static const string query_init{
        "nvidia-smi --query-gpu=name,memory.total,power.max_limit --format=csv "
        "-i " +
        to_string(gpu_id) + " | tail -n 1"};
    inline static const string query_update{
        "nvidia-smi "
        "--query-gpu=fan.speed,utilization.gpu,utilization.memory,"
        "temperature.gpu,power.draw,clocks.gr,clocks.mem --format=csv -i " +
        to_string(gpu_id) + " | tail -n 1"};

    /// Data only queried once per run.
    string name;

    /// GiB
    float memory_total;

    /// Watts
    short power_max;

    /// Data queried every update.
    /// %
    u_char fan;

    /// %
    u_char utilization;
    u_char memory;

    /// Celsius
    u_char temp;

    /// Watts
    float power;

    /// MHz
    short clock_gpu;
    short clock_mem;

    Nvidia() {
        ExecutorCSV e(query_init);

        name = e.at(0);
        memory_total = (float)stoi(e.at(1)) / 1000;
        power_max = static_cast<short>(stoi(e.at(2)));

        update();
    }

    void update() {
        ExecutorCSV e(query_update);
        fan = stoi(e.at(0));
        utilization = stoi(e.at(1));
        memory = stoi(e.at(2));
        temp = stoi(e.at(3));
        power = stof(e.at(4));
        clock_gpu = static_cast<short>(stoi(e.at(5)));
        clock_mem = static_cast<short>(stoi(e.at(6)));
    }

    ostream &print(ostream &os) const {
        os << "Static: {" << nl;
        {
            dbg::IndentGuard ig{};
            os << "name: " << name << nl;
            os << "memory_total: " << memory_total << " GiB" << nl;
            os << "power_max: " << power_max << " W" << nl;
        }
        os << "}," << nl;
        os << "Dynamic: {" << nl;
        {
            dbg::IndentGuard ig{};
            os << "Fan speed: " << +fan << " %" << nl;
            os << "Utilization : " << +utilization << " %" << nl;
            os << "Memory Utilization: " << +memory << " %" << nl;
            os << "Temp: " << +temp << " C" << nl;
            os << "Power: " << power << " W" << nl;
            os << "GPU clock: " << clock_gpu << " MHz" << nl;
            os << "Memory clock: " << clock_mem << " MHz" << nl;
        }
        os << "}," << nl;

        return os;
    }
};
}; // namespace fprd::query