/// @file CPU.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Theme.hpp>
#include <fprd/Threads.hpp>
#include <fprd/Window.hpp>
#include <fprd/draw/Text.hpp>
#include <fprd/draw/animated/ABar.hpp>
#include <fprd/draw/animated/AGraph.hpp>
#include <fprd/probes/CPU.hpp>
#include <fprd/util/AnimatedValue.hpp>
#include <fprd/util/ranges.hpp>

#include "fprd/draw/animated/AnimatedList.hpp"

namespace fprd {
using namespace std;
class CPU {
    static constexpr auto max_procs{16};
    using Probe = probe::CPU<max_procs>;
    using ProcList = AnimatedList<typename Probe::Process, max_procs>;

   public:
    using DynamicData = typename Probe::DynamicData;
    static constexpr auto probe_interval{1s};

    static constexpr auto w{256};
    static constexpr auto cores_row{theme::medium_area(w)};
    static constexpr auto cores_rows{4};
    static constexpr Area<float> graph_area{w, 32};
    static constexpr Area<float> procs_area{w,
                                            (max_procs + 1) * theme::small_h};

    static constexpr Area<float> area{w, cores_row.h* cores_rows +
                                             procs_area.h + theme::large_h + 3 +
                                             graph_area.h * 2};

    const Position<int> pos;

   private:
    Probe probe;

    vector<AnimatedBar<Orientation::horizontal, Direction::positive>>
        core_usages;
    vector<Text<VerticalAlign::center>> core_freqs;
    vector<AnimatedValue<ushort>> core_freqs_v;
    AnimatedGraph<128> usage;
    AnimatedValue<float> temp_v{};
    Text<VerticalAlign::center> temp;
    AnimatedGraph<128> memory;
    AnimatedValue<int> memory_v{};
    Text<VerticalAlign::center> memory_value;
    const string total_memory;
    unique_ptr<ProcList> procs;

   public:
    CPU(Position<int> pos)
        : pos{pos},
          probe{},
          usage{{{0, theme::large_h + 3 + cores_row.h * cores_rows},
                 graph_area,
                 theme::grey,
                 1,
                 theme::red,
                 theme::black}},
          memory{{{0, theme::large_h + 3 + cores_row.h * cores_rows +
                          graph_area.h},
                  graph_area,
                  theme::grey,
                  1,
                  theme::green,
                  theme::black}},
          total_memory{"/" + ftos<1>((float)probe.mem_total / 1000000) + "GB"} {
    }

    void update_data(const DynamicData& d) {
        for (auto [ts, b, f] : zip(d.threads, core_usages, core_freqs_v)) {
            b.update(ts.usage * 100);
            f.update(ts.freq);
        }
        usage.update(d.avg.usage * 100);

        const auto mem_usage{static_cast<float>(probe.mem_total - d.mem_free) /
                             static_cast<float>(probe.mem_total)};
        memory.update(mem_usage * 100);
        memory_v.update(probe.mem_total - d.mem_free);
        temp_v.update(d.temp);

        procs->update(d.procs);
    }

    void draw(Window& w, bool new_data) {
        for (auto& b : core_usages) {
            b.draw(w);
        }
        for (auto [v, f] : zip(core_freqs_v, core_freqs)) {
            f.draw(w, width<4>(to_string(v.draw())) + "MHz");
        }
        usage.draw(w);
        memory.draw(w);
        procs->draw(w);
        memory_value.draw(
            w, ftos<3>((float)memory_v.draw() / 1000000) + total_memory);
        temp.draw(w, ftos<1>(temp_v.draw()) + "℃");
    }

    DynamicData get_data() { return probe.update(); };

    Window create_window() {
        Window w{":0.0", pos, area};

        Text<VerticalAlign::center> t{
            {&theme::bold, {0, 0}, theme::large_area(area.w)}, theme::red};
        draw_text_once(w, t, [&] {
            const auto name{probe.name};
            const string_view start{"Core(TM)"};
            const auto spos{name.find(start) + start.size()};
            const auto epos{name.find("CPU") - 1};
            return "Intel" + name.substr(spos, epos - spos);
        }());

        const auto cores_per_row{probe.thread_count / cores_rows};

        Margin<float> m{1, 1};
        const Area<float> core_area{
            cores_row.scale({1.0F / cores_per_row, 1.0F})};
        Bar<Orientation::horizontal, Direction::positive> bbase{
            .area = core_area.pad(m),
            .border_width = 1,
            .frame = theme::grey,
            .empty = theme::black,
            .filled = theme::red,
        };
        Text<VerticalAlign::center> tc{{&theme::normal, {}, core_area.pad(m)},
                                       theme::white};
        for (auto i{0U}; i < probe.thread_count; i++) {
            const auto x{i / cores_per_row};
            const auto y{i % cores_per_row};

            const Position<float> pos{x * core_area.w,
                                      y * core_area.h + theme::large_h + 3};
            bbase.pos = pos.pad(m);
            core_usages.emplace_back(bbase);
            tc.pos = pos;
            core_freqs.emplace_back(tc);
        }
        core_freqs_v.resize(probe.thread_count);

        memory_value = tc;
        memory_value.area = theme::medium_area(area.w);
        memory_value.pos = memory_value.area.vertical_center(
            Position<double>{0, theme::large_h + 3 + core_area.h * cores_rows +
                                    graph_area.h * 1.5});

        temp = memory_value;
        temp.pos = temp.area.vertical_center(
            Position<double>{0, theme::large_h + 3 + core_area.h * cores_rows +
                                    graph_area.h * 0.5});

        procs = make_unique<ProcList>(
            w,
            Position<float>{0, cores_rows * core_area.h + theme::large_h + 3 +
                                   graph_area.h * 2},
            procs_area);

        return w;
    };
};

class CPUWindow {
    CPU c;
    Threads<CPU> t;

   public:
    static constexpr Area<int> area{CPU::area};

    CPUWindow(atomic<bool>& run, Position<int> pos) : c{pos}, t{run, c} {}
};
};  // namespace fprd