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

template <uint gpu_id> struct NvidiaProcesses {
  private:
    inline static const string query{"nvidia-smi -q -d PIDS -i " +
                                     to_string(gpu_id)};

  public:
    /// Maximum number of processes shown.
    static const u_char max{5};
    /// Maximum length of the process name.
    static const u_char max_name_len{22};

    /// A GPU process
    struct Process {
        ulong id;
        char type;
        array<char, max_name_len> name;
        ushort memory; // MiB

        /// If the id is non-zero, it means it's a valid process.
        /// @return true
        /// @return false
        [[nodiscard]] bool is_valid() const { return id != 0; }

        auto operator<=>(const Process &rhs) const {
            return memory <=> rhs.memory;
        };

        ostream &print(ostream &os) const {
            if (is_valid()) {
                os << "id: " << id << ", type: " << type
                   << ", name: " << string{name.data()}
                   << ", Memory: " << memory << " MiB";
            } else {
                os << "<invalid>";
            }
            return os;
        }
    };

    array<Process, max> processes;
    vector<Process> proc_buf;

    NvidiaProcesses() : processes{} {}

    void update() {
        ExecutorLine e{query};
        auto line{13};
        proc_buf.resize(0);

        for (;;) {
            const auto p{[&]() {
                Process p;
                p.id = stoul(e.get_line(line).substr(44, 5));
                p.type = e.get_line(line + 1)[44];
                p.name = [&]() -> array<char, max_name_len> {
                    const auto s{e.get_line(line + 2)};
                    const auto s_begin{s.begin() + 44};
                    const auto s_end{find(s_begin, s.end(), ' ')};
                    const auto size{s_end - s_begin};
                    array<char, max_name_len> a{};
                    if (size <= max_name_len) {
                        copy(s_begin, s.end(), a.data());
                    } else {
                        copy(s_end - max_name_len + 3, s_end, a.data());
                        a[0] = '.';
                        a[1] = '.';
                        a[2] = '.';
                    }
                    return a;
                }();
                p.memory = stoul(e.get_line(line + 3).substr(44, 10));
                return p;
            }()};

            proc_buf.insert(upper_bound(proc_buf.begin(), proc_buf.end(), p,
                                        greater<Process>()),
                            p);

            line += 6;
            if (e.get_line(line - 1), e == false) {
                break;
            }
        }
        if (proc_buf.size() >= max) {
            copy(proc_buf.begin(), proc_buf.begin() + max, processes.data());
        } else {
            copy(proc_buf.begin(), proc_buf.end(), processes.data());
            for_each(processes.begin() + proc_buf.size(), processes.end(),
                     [](auto &p) { p.id = 0; });
        }
    }
};

template <uint gpu_id> struct Nvidia {
  private:
    inline static const string query_init{
        "nvidia-smi --query-gpu=name,memory.total,power.max_limit --format=csv "
        "-i " +
        to_string(gpu_id) + " | tail -n 1"};
    inline static const string query_update{
        "nvidia-smi "
        "--query-gpu=fan.speed,utilization.gpu,utilization.memory,"
        "temperature.gpu,power.draw,clocks.gr,clocks.mem --format=csv -i " +
        to_string(gpu_id) + " | tail -n 1"};

  public:
    NvidiaProcesses<gpu_id> procs;

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
        procs.update();
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

    Nvidia(const Nvidia &) = delete;
    Nvidia(Nvidia &&) noexcept = default;
};
}; // namespace fprd::query