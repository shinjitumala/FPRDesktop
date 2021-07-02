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
#include <thread>

namespace fprd {
using namespace std;

namespace cpu {
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

/// Information that have to be sampled from the DynamicInformation.
struct SampledInfo {
    long double util;                               // Ratio [0, 1]
    llvm::SmallVector<long double, 16> thread_util; // Ratio [0, 1]
    long double mem;                                // Ratio [0, 1]
    long double mem_used;                           // GB
};

class Widget {
  public:
    static constexpr int width{64};
    static constexpr int height{36};

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
            .mem_hist = {buf, {0, 17}, {width, 27}},
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
        buf.printf(16, "Memory:        / %3.0LfGB", ci.mem_total * 1e-6L);
        buf.printf(27, "Temp:     ℃");
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
            update_state_info(info, cur, prev, sampled);
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
        buf.printf_st(16, 8, 15, "%6.2Lf", si.mem_used);

        // di.temp
        buf.printf_st(27, 6, 10, "%3d", di.temp);
    }

    /// Called every frame to update the SampledInfo, which requires processing the DynamicInfo from the previous
    /// frame.
    /// @param si
    /// @param di
    /// @param prev
    /// @param state
    static auto update_state_info(const ConstInfo &ci, const DynamicInfo &cur, const PrevDynamicInfo &prev,
                                  SampledInfo &sampled) -> void {
        static auto get_util{[](const probe::cpu::Stat::Line &prev, const probe::cpu::Stat::Line &cur) {
            const auto idle{cur.idle - prev.idle};
            const auto active{(cur.user - prev.user) + (cur.user - prev.user)};

            return (long double)active / (active + idle);
        }};
        sampled.util = get_util(prev.stat.cpu, cur.stat.cpu);
        tie(sampled.mem, sampled.mem_used) = [&] {
            const auto used_kb{ci.mem_total - cur.mem.free}; // KB
            const auto used_gb{used_kb * 1e-6L};             // GB
            const auto used_ratio{(long double)used_kb / ci.mem_total};
            return make_pair(used_ratio, used_gb);
        }();
        for (auto [p, c, r] : fprd::zip(prev.stat.threads, cur.stat.threads, sampled.thread_util)) {
            r = get_util(p, c);
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