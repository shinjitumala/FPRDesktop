/// @file CPU.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Theme.hpp>
#include <fprd/Window.hpp>
#include <fprd/element/Bar.hpp>
#include <fprd/element/Graph.hpp>
#include <fprd/probes/CPU.hpp>
#include <fprd/probes/NVML.hpp>
#include <fprd/util/ranges.hpp>
#include <llvm/ADT/SmallString.h>
#include <thread>

namespace fprd {
using namespace std;

namespace cpu {
constexpr auto max_procs{32};

/// Storage type for data that requires storing the previous value to present.
struct PrevDynamicInfo {
    probe::cpu::Stat stat;
    probe::cpu::Procs procs;
};

/// Storage required for drawing.
template <class Window> struct TDrawState {
    element::HGraph<element::graph_standard, Window> util_hist;
    llvm::SmallVector<element::Bar<element::bar_standard, Window>, 16> thread_util;
    element::HGraph<element::graph_standard, Window> mem_hist;
};

struct Proc {
    pid_t pid;
    char mode;
    mutable llvm::SmallString<32> name;
    long usage;
    unsigned long mem; // KB

    Proc() = default;

    Proc(const probe::cpu::Proc &p, long usage) : pid{p.pid}, mode{p.state}, usage{usage}, mem{p.mem} {
        get_name(pid, name);
    }

    auto operator<(const Proc &o) const -> auto { return usage > o.usage; }
};

/// Information that have to be sampled from the DynamicInformation.
struct SampledInfo {
    unsigned long cpu_time;                         // Total cycles for all threads.
    long double util;                               // Ratio [0, 1]
    llvm::SmallVector<long double, 16> thread_util; // Ratio [0, 1]
    long double mem;                                // Ratio [0, 1]
    long double mem_used;                           // GB
    llvm::SmallVector<Proc, 512> procs;
};

class Widget {
  public:
    static constexpr int width{64};
    static constexpr int height{33 + max_procs};

    static constexpr int left_end{21};

  private:
    using Window = fprd::Window<width, height>;
    using Buffer = Window::Lines;
    using DrawState = TDrawState<Window>;
    using ConstInfo = probe::cpu::StaticInfo;
    using DynamicInfo = probe::cpu::DynamicInfo;

    Window w;
    Buffer buf;

    const ConstInfo info;

    PrevDynamicInfo prev;
    DynamicInfo cur;

    SampledInfo sampled;

    DrawState draw;

    atomic<bool> run;
    thread t;

    /// Initialize the DrawState.
    /// The positions of the drawing "element"s are adjusted here.
    /// @param buf
    /// @param si
    /// @return auto
    static auto init_drawstate(Buffer &buf, const ConstInfo &si) -> auto {
        return DrawState{
            .util_hist = {buf, {0, 2}, {width, 12}},
            .thread_util =
                [&]() {
                    decltype(DrawState::thread_util) bars;
                    const auto bars_per_line{4};
                    const auto bar_w{width / bars_per_line};
                    auto line{12};
                    for (auto tidx{0}; tidx < si.threads;) {
                        for (auto x{0}; x < bars_per_line; x++) {
                            bars.emplace_back(buf, line, x * bar_w, (x + 1) * bar_w);
                            tidx++;
                        }
                        line++;
                    }

                    return bars;
                }(),
            .mem_hist = {buf, {0, 18}, {width, 28}},
        };
    };

    /// The startup initialization of the contents of the buffer is here.
    /// @param buf
    /// @param si
    /// @return auto
    static auto init_buf(Buffer &buf, const ConstInfo &ci) {
        // CPU name at the header.
        const auto namesize{ci.name.size()};
        const auto lmargin{(width - namesize) / 2};
        buf.printf(0, "%*c%s", lmargin, ' ', ci.name.data());
        buf.printf(1, "Utilization:        ");
        buf.printf(17, "Memory:         / %3.0LfGB", ci.mem_total * 1e-6L);
        buf.printf(29, "Temp:     ℃");
        buf.printf(31, "Processes");
    }

  public:
    Widget(Position<float> pos)
        : w{"", pos}, info{probe::cpu::get_info()}, draw{init_drawstate(buf, info)}, run{true}, t{[this] {
              runner();
          }} {}
    ~Widget() { t.join(); }

    /// Request the widget to stop. The deconstructor should stop blocking shortly after a call to this.
    auto stop() -> void { run = false; }

  private:
    auto runner() -> void {
        w.update(buf); // Update once to make the window background appar immediately.

        int thread_count{info.threads};
        sampled.thread_util.resize(thread_count);

        init_buf(buf, info);

        // The initial probing.
        probe::cpu::update_data(cur, thread_count);
        update_prev(cur, prev);

        while (run) {
            auto tp{now()};

            w.update(buf);
            probe::cpu::update_data(cur, thread_count);
            update_sample(info, cur, prev, sampled);
            update_buf(sampled, cur, draw, buf);
            update_prev(cur, prev);

            this_thread::sleep_until(tp + 1s);
        }
    }

    /// Called every frame to update the contents of the buffer to the current state.
    /// @param si
    /// @param di
    /// @param sd
    /// @param buf
    static auto update_buf(const SampledInfo &si, const DynamicInfo &di, DrawState &draw, Buffer &buf) -> void {
        // Utilization
        buf.printf_st(1, 13, width, "%6.2Lf%%", si.util * 100);
        draw.util_hist.update(buf, si.util);

        // Thread utilzation
        for (auto [b, d] : fprd::zip(draw.thread_util, si.thread_util)) {
            b.update(buf, d);
        }

        // Memory
        draw.mem_hist.update(buf, si.mem);
        buf.printf_st(17, 8, 16, "%7.3Lf", si.mem_used);

        // Temperature
        buf.printf_st(29, 6, 10, "%3d", di.temp);

        // Processes
        for (auto i{0}; i < max_procs; i++) {
            buf.printf(32 + i, "%*c", width, ' ');
        }
        for (auto [idx, p] : si.procs | enumerate) {
            buf.printf(32 + idx, "%-6d %c %32.32s %6.2Lf%% %12.0LfMB", p.pid, p.mode, p.name.c_str(),
                       (long double)p.usage / si.cpu_time * 100.0L, p.mem * 1e-6L);
        }
    }

    /// Called every frame to update the SampledInfo, which requires processing the DynamicInfo from the previous
    /// frame.
    /// @param si
    /// @param di
    /// @param prev
    /// @param state
    static auto update_sample(const ConstInfo &ci, const DynamicInfo &cur, const PrevDynamicInfo &prev,
                              SampledInfo &sampled) -> void {
        static auto get_util{[](const probe::cpu::Stat::Line &prev, const probe::cpu::Stat::Line &cur) {
            const auto idle{cur.idle - prev.idle};
            const auto active{(cur.user - prev.user) + (cur.user - prev.user)};

            return make_pair((long double)active / (active + idle), active + idle);
        }};
        sampled.util = get_util(prev.stat.cpu, cur.stat.cpu).first;
        tie(sampled.mem, sampled.mem_used) = [&] {
            const auto used_kb{ci.mem_total - cur.mem.free}; // KB
            const auto used_gb{used_kb * 1e-6L};             // GB
            const auto used_ratio{(long double)used_kb / ci.mem_total};
            return make_pair(used_ratio, used_gb);
        }();
        sampled.cpu_time = 0;
        for (auto [p, c, r] : fprd::zip(prev.stat.threads, cur.stat.threads, sampled.thread_util)) {
            unsigned long u;
            tie(r, u) = get_util(p, c);
            sampled.cpu_time += u;
        }

        // Processes
        sampled.procs.clear();
        for (const auto &p : prev.procs) {
            const auto *const itr{find_if(cur.procs, [&p](auto &c) { return p.pid == c.pid; })};
            if (itr == cur.procs.end()) {
                // Ignore if 'p' did not exist previously.
                continue;
            }
            const auto &c{*itr};
            const auto usage{c.usage - p.usage};
            if (usage <= 0) {
                // Ignore if usage is below 0.
                continue;
            }

            sampled.procs.emplace_back(c, usage);
        }
        sort(sampled.procs.begin(), sampled.procs.end());
        const auto procs_count{sampled.procs.size()};
        if (max_procs < procs_count) {
            sampled.procs.resize(max_procs);
        }
    }

    /// Update PrevDynamicInfo with the current DynamicInfo.
    /// @param di
    /// @param prev
    static auto update_prev(const DynamicInfo &cur, PrevDynamicInfo &prev) -> void {
        prev.stat = cur.stat;
        prev.procs = cur.procs;
    }
};
}; // namespace cpu
}; // namespace fprd